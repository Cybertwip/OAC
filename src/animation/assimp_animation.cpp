#include "assimp_animation.h"
#include "../util/utility.h"

#include "SmallFBX.h"

namespace fs = std::filesystem;

namespace anim
{
FbxAnimation::FbxAnimation(const sfbx::DocumentPtr doc, const std::shared_ptr<sfbx::AnimationLayer> animationLayer, const std::string& path)
{
	init_animation(doc, animationLayer, path);
}

FbxAnimation::~FbxAnimation()
{
}

void FbxAnimation::init_animation(const sfbx::DocumentPtr doc, const std::shared_ptr<sfbx::AnimationLayer> animationLayer, const std::string& path)
{
	type_ = AnimationType::Fbx;
	
	path_ = path;
	fs::path anim_path = fs::u8path(path_);
	name_ = anim_path.filename().string();
	
	process_bones(animationLayer.get(), doc->getRootModel().get());
	
	

	float duration = 0.0f;
	for (auto &name_bone : name_bone_map_)
	{
		
		if(name_bone.second->get_time_set().size() > 0){
			auto time_end = *std::next(name_bone.second->get_time_set().end(), -1);
			duration = std::max(duration, time_end);
		}
	}
	
	duration_ = duration;
	fps_ = 60;

	process_bindpose(doc->getRootModel().get());
}

void FbxAnimation::process_bones(const sfbx::AnimationLayer *animation, sfbx::Model *node)
{
	auto curveNodes = animation->getAnimationCurveNodes();
	
	auto rotationCondition = [&curveNodes, &node](const std::shared_ptr<sfbx::AnimationCurveNode> curveNode) {
		return curveNode->getAnimationKind() == sfbx::AnimationKind::Rotation && curveNode->getAnimationTarget().get() == node;
	};

	auto positionCondition = [&curveNodes, &node](const std::shared_ptr<sfbx::AnimationCurveNode> curveNode) {
		return curveNode->getAnimationKind() == sfbx::AnimationKind::Position && curveNode->getAnimationTarget().get() == node;
	};
	
	auto scaleCondition = [&curveNodes, &node](const std::shared_ptr<sfbx::AnimationCurveNode> curveNode) {
		return curveNode->getAnimationKind() == sfbx::AnimationKind::Scale && curveNode->getAnimationTarget().get() == node;
	};

	auto positionCurveIter = std::find_if(curveNodes.begin(), curveNodes.end(), positionCondition);

	auto rotationCurveIter = std::find_if(curveNodes.begin(), curveNodes.end(), rotationCondition);

	auto scaleCurveIter = std::find_if(curveNodes.begin(), curveNodes.end(), scaleCondition);

	std::shared_ptr<sfbx::AnimationCurveNode> positionCurve = nullptr;
	std::shared_ptr<sfbx::AnimationCurveNode> rotationCurve = nullptr;
	std::shared_ptr<sfbx::AnimationCurveNode> scaleCurve = nullptr;
	
	
	if(positionCurveIter != curveNodes.end()){
		positionCurve = *positionCurveIter;
	}
	
	if(rotationCurveIter != curveNodes.end()){
		rotationCurve = *rotationCurveIter;
	}
	
	if(scaleCurveIter != curveNodes.end()){
		scaleCurve = *scaleCurveIter;
	}
	
	auto bone_name = std::string{node->getName()};
		
	auto poseMatrix = SfbxMatToGlmMat(node->getLocalMatrix());
	
	name_bone_map_[bone_name] = std::make_unique<Bone>(bone_name, positionCurve.get(), rotationCurve.get(), scaleCurve.get(), glm::inverse(poseMatrix));
	
	for(auto& child : node->getChildren()){
		if(auto model = sfbx::as<sfbx::Model>(child); model){
			process_bones(animation, model.get());
		}
	}
}

void FbxAnimation::process_bindpose(sfbx::Model *node)
{
	if (node)
	{
		std::string name(node->getName());
		auto bindpose = SfbxMatToGlmMat(node->getLocalMatrix());
		auto children_size = node->getChildren().size();
		name_bindpose_map_[name] = bindpose;
		for (unsigned int i = 0; i < children_size; ++i)
		{
			process_bindpose(sfbx::as<sfbx::Model>(node->getChildren()[i]).get());
		}
	}
}
}
