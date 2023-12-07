#pragma once
#include "sfbxObject.h"

namespace sfbx {

// animation-related classes
// (AnimationStack, AnimationLayer, AnimationCurveNode, AnimationCurve)

enum class AnimationKind
{
    Unknown,

    // transform
    Position,    // float3
    Rotation,    // float3
    Scale,       // float3

    // light
    Color,       // float3
    Intensity,   // float

    // camera
    FocalLength, // float, in mm
    FilmWidth,   // float, in mm (unit converted. raw value is inch)
    FilmHeight,  // float, in mm (unit converted. raw value is inch)
    FilmOffsetX, // float, in mm (unit converted. raw value is inch)
    FilmOffsetY, // float, in mm (unit converted. raw value is inch)

    // blend shape
    DeformWeight, // float, 0.0-1.0 (unit converted. raw value is percent)

    // internal
    filmboxTypeID,
    lockInfluenceWeights,
};

class AnimationStack : public Object
{
using super = Object;
public:
    ObjectClass getClass() const override;
    void addChild(ObjectPtr v) override;
    void eraseChild(ObjectPtr v) override;

    float getLocalStart() const;
    float getLocalStop() const;
    float getReferenceStart() const;
    float getReferenceStop() const;
    span<std::shared_ptr<AnimationLayer>> getAnimationLayers() const;

    std::shared_ptr<AnimationLayer> createLayer(string_view name = {});

    void applyAnimation(float time);

    bool remap(Document* doc);
    bool remap(DocumentPtr doc) { return remap(doc.get()); }
    void merge(AnimationStack* src); // merge src into this

protected:
    void importFBXObjects() override;
    void exportFBXObjects() override;

    float m_local_start{};
    float m_local_stop{};
    float m_reference_start{};
    float m_reference_stop{};
    std::vector<std::shared_ptr<AnimationLayer>> m_anim_layers;
	
	friend class Document;

};


class AnimationLayer : public Object
{
using super = Object;
public:
    ObjectClass getClass() const override;
    void addChild(ObjectPtr v) override;
    void eraseChild(ObjectPtr v) override;

    span<std::shared_ptr<AnimationCurveNode>> getAnimationCurveNodes() const;

	std::shared_ptr<AnimationCurveNode> createCurveNode(AnimationKind kind, ObjectPtr target);

    void applyAnimation(float time);

    bool remap(Document* doc);
    void merge(std::shared_ptr<AnimationLayer> src);

protected:
    void exportFBXObjects() override;

    std::vector<std::shared_ptr<AnimationCurveNode>> m_anim_nodes;
};


class AnimationCurveNode : public Object
{
using super = Object;
public:
    ObjectClass getClass() const override;
    void addChild(ObjectPtr v) override;
    void addChild(ObjectPtr v, string_view p) override;
    void eraseChild(ObjectPtr v) override;

    AnimationKind getAnimationKind() const;
    ObjectPtr getAnimationTarget() const;
    span<std::shared_ptr<AnimationCurve>> getAnimationCurves() const;
    float getStartTime() const;
    float getStopTime() const;

    // evaluate curve(s)
    float evaluateF1(float time) const;
    float3 evaluateF3(float time) const;
    int evaluateI(float time) const;

    // apply evaluated value to target
    void applyAnimation(float time) const;

    void addValue(float time, float value);
    void addValue(float time, float3 value);

    bool remap(Document* doc);
    void unlink();

protected:
    friend class AnimationLayer;
    void importFBXObjects() override;
    void exportFBXObjects() override;
    void exportFBXConnections() override;
    void setup(AnimationKind kind, ObjectPtr target, bool create_curves);

    AnimationKind m_kind = AnimationKind::Unknown;
    std::vector<std::shared_ptr<AnimationCurve>> m_curves;

    union {
        Object* object;
        Model* model;
        Light* light;
        Camera* camera;
        BlendShapeChannel* bs_channel;
    } m_target{};

    union {
        float3 f3;
        int i;
    } m_default_value{};
};


class AnimationCurve : public Object
{
using super = Object;
friend class AnimationCurveNode;
public:
    ObjectClass getClass() const override;

    float getUnitConversion() const;
    span<float> getTimes() const;
    span<float> getRawValues() const; // get values without unit conversion
    float getStartTime() const;
    float getStopTime() const;
    float evaluate(float time) const;

    void setUnitConversion(float v); // conversion coefficient applied when *getting* values
    void setTimes(span<float> v);
    void setRawValues(span<float> v); // set values without unit conversion
    void addValue(float time, float value);

protected:
    void importFBXObjects() override;
    void exportFBXObjects() override;
    void exportFBXConnections() override;
    void setLinkName(string_view v);
    void setElementIndex(int v);

    float m_default{};
    RawVector<float> m_times;
    RawVector<float> m_values;

    std::string m_link_name;
    int m_element_index{};
    float m_unit_conversion = 1.0f;
    float m_unit_conversion_rcp = 1.0f;
};

} // namespace sfbx
