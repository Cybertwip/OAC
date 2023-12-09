#pragma once
#include "sfbxObject.h"

#include <sstream>
#include <vector>

namespace sfbx {

// texture & material

// Video represents image data
class Video : public Object
{
using super = Object;
public:
    ObjectClass getClass() const override;
    ObjectSubClass getSubClass() const override;
	
	bool getEmbedded() const;
	std::string_view getFilename() const;
	const std::vector<uint8_t>& getData() const;
	
protected:
	void importFBXObjects() override;
	void exportFBXObjects() override;

private:
	std::vector<std::stringstream> mChildStreams;
	
	bool m_embedded;
	std::string m_filename;
	std::vector<uint8_t> m_data;
};

class Texture : public Object
{
using super = Object;
public:
    ObjectClass getClass() const override;

	
	bool getEmbedded() const;
	std::string_view getFilename() const;
	const std::vector<uint8_t>& getData() const;

protected:
	void importFBXObjects() override;
	void exportFBXObjects() override;
	void exportFBXConnections() override;

private:
	std::vector<std::stringstream> mChildStreams;
	
	bool m_embedded;
	std::string m_filename;
	std::vector<uint8_t> m_data;

};

class Material : public Object
{
using super = Object;
public:
    ObjectClass getClass() const override;

	double3 getAmbientColor() const;
	double3 getDiffuseColor() const;
	double3 getSpecularColor() const;
	float64 getShininess() const;
	
	std::shared_ptr<sfbx::Texture> getTexture(const std::string& textureType);
	
protected:
    void importFBXObjects() override;
    void exportFBXObjects() override;
	void exportFBXConnections() override;
	
private:
	double3 m_ambient_color;
	double3 m_diffuse_color;
	double3 m_specular_color;
	float64 m_shininess;
	
	std::vector<std::stringstream> mChildStreams;
	
	std::unordered_map<std::string, std::shared_ptr<sfbx::Texture>> m_textures;
};


class Implementation : public Object
    {
using super = Object;
public:
    ObjectClass getClass() const override;
};

class BindingTable : public Object
{
using super = Object;
public:
    ObjectClass getClass() const override;
};

} // namespace sfbx
