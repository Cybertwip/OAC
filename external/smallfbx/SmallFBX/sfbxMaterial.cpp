#include "sfbxInternal.h"
#include "sfbxMaterial.h"
#include "sfbxDocument.h"

#include <iostream>
#include <fstream>
#include <vector>

namespace {
	const std::string kEmbeddedToken = "*";


std::string getFileExtension(const std::string& filename) {
	// Find the last dot in the filename
	size_t dotPos = filename.find_last_of('.');
	
	// Check if a dot was found and it's not the last character in the string
	if (dotPos != std::string::npos && dotPos < filename.length() - 1) {
		// Extract and return the substring after the last dot
		return filename.substr(dotPos + 1);
	} else {
		// No dot found or it's the last character, return an empty string or handle as needed
		return "";
	}
}
}

namespace sfbx {

ObjectClass Video::getClass() const { return ObjectClass::Video; }
ObjectSubClass Video::getSubClass() const { return ObjectSubClass::Clip; }

bool Video::getEmbedded() const {
	return m_embedded;
}

std::string_view Video::getFilename() const {
	return m_filename;
}

const std::vector<uint8_t>& Video::getData() const {
	return m_data;
}

void Video::importFBXObjects()
{
	super::importFBXObjects();
	// todo

	std::string embeddedFilename;
	std::string imageExtension;

	bool isEmbedded = false;
	
	for(auto& child : getNode()->getChildren()){

		if(child->getName() == "Filename"){
			embeddedFilename = child->getProperty(0)->getString();
		}
				
		if(child->getName() == "Content"){
			isEmbedded = true;
			auto content = child->getProperty(0)->getString();
			m_data = std::vector<uint8_t>(content.begin(), content.end()) ;
		}

	}
	
	
	if(isEmbedded){
		imageExtension = getFileExtension(embeddedFilename);
		embeddedFilename = kEmbeddedToken + ":" + std::to_string(this->getID()) + "." + imageExtension;
	}
	
	for(auto& child : getNode()->getChildren()){
		auto stream = std::stringstream();
		
		if(child->getName() == "Filename" || child->getName() == "RelativeFilename"){
			if(isEmbedded){
				child->getProperty(0)->assign(embeddedFilename);
			}
		}
		
		child->writeBinary(stream, 0);

		mChildStreams.push_back(std::move(stream));
	}
	
	m_embedded = isEmbedded;
	m_filename = embeddedFilename;
	
	if(!m_embedded){
		// Open the file in binary mode
		std::ifstream file(m_filename, std::ios::binary);
		
		// Check if the file is successfully opened
		if (file.is_open()) {
			// Read the file contents into a vector<uint8_t>
			m_data = std::vector<uint8_t>(
										  (std::istreambuf_iterator<char>(file)),
										  std::istreambuf_iterator<char>()
										  );
			
			// Close the file
			file.close();
			
		} else {
			std::cerr << "Error opening file: " << m_filename << std::endl;
		}
	}
}


void Video::exportFBXObjects()
{
	super::exportFBXObjects();
	
	for(auto& stream : mChildStreams){
		auto child = getNode()->createChild();

		stream.seekg(std::ios::beg);
		
		child->readBinary(stream, 0);
	}
}

ObjectClass Texture::getClass() const { return ObjectClass::Texture; }


bool Texture::getEmbedded() const {
	return m_embedded;
}

std::string_view Texture::getFilename() const {
	return m_filename;
}

const std::vector<uint8_t>& Texture::getData() const {
	return m_data;
}

void Texture::importFBXObjects()
{
	super::importFBXObjects();
	// todo
	
	auto filter = find_if(m_children, [](ObjectPtr ptr){
		return sfbx::as<Video>(ptr);
	});
	
	auto video = sfbx::as<Video>(filter);
	
	bool hasVideo = false;
	
	std::string embeddedFilename;
	
	if(video){
		hasVideo = true;
		
		m_embedded = video->getEmbedded();
		
		m_filename = video->getFilename();
	}
	
	
	for(auto& child : getNode()->getChildren()){
		if(child->getName() == "FileName" || child->getName() == "RelativeFilename"){
			
			if(hasVideo){
				if(m_embedded){
					child->getProperty(0)->assign(m_filename);
				}
			}
		}
		
		if(child->getName() == "FileName"){
			if(!m_embedded){
				m_filename = child->getProperty(0)->getString();
			}
		}

		auto stream = std::stringstream();
		
		child->writeAscii(stream);
		
		mChildStreams.push_back(std::move(stream));
	}
	
	
	if(m_embedded){
		m_data = video->getData();
	} else {
		if(hasVideo){
			m_data = video->getData();
		} else {
			// Open the file in binary mode
			std::ifstream file(m_filename, std::ios::binary);
			
			// Check if the file is successfully opened
			if (file.is_open()) {
				// Read the file contents into a vector<uint8_t>
				m_data = std::vector<uint8_t>(
											   (std::istreambuf_iterator<char>(file)),
											   std::istreambuf_iterator<char>()
											   );
				
				// Close the file
				file.close();

			} else {
				std::cerr << "Error opening file: " << m_filename << std::endl;
			}
		}
	}
}

void Texture::exportFBXObjects()
{
	super::exportFBXObjects();
	
	for(auto& stream : mChildStreams){
		auto child = getNode()->createChild();
		auto streamString = stream.str();
		auto streamView = std::string_view(streamString);
		child->readAscii(streamView);
	}
}

void Texture::exportFBXConnections()
{
	// ignore super::constructLinks()
	
	for(auto& parent : getParents()){
		m_document->createLinkOO(shared_from_this(), parent);
	}
}

ObjectClass Material::getClass() const { return ObjectClass::Material; }

double3 Material::getAmbientColor() const
{
	return m_ambient_color;
}

double3 Material::getDiffuseColor() const
{
	return m_diffuse_color;
}


double3 Material::getSpecularColor() const
{
	return m_specular_color;
}

float64 Material::getShininess() const
{
	return m_shininess;
}

std::shared_ptr<sfbx::Texture> Material::getTexture(const std::string& textureType){
	return m_textures[textureType];
}

void Material::importFBXObjects()
{
    super::importFBXObjects();
	
    // todo
	for(auto& child : getNode()->getChildren()){
		
		if(child->getName() == "Properties70"){
			
			auto propertyChildren = child->getChildren();
			
			for(auto propertyChild : propertyChildren){
				auto name = propertyChild->getProperty(0)->getString();
				
				
				if(name == "AmbientColor"){
					propertyChild->getPropertiesValues<double3>(4, m_ambient_color);
				}
				
				if(name == "DiffuseColor"){
					propertyChild->getPropertiesValues<double3>(4, m_diffuse_color);
				}

				if(name == "SpecularColor"){
					propertyChild->getPropertiesValues<double3>(4, m_specular_color);
				}
				
				if(name == "Shininess"){
					m_shininess = propertyChild->getProperty(4)->getValue<float64>();
				}
			}
		}
			
		
		auto stream = std::stringstream();
		
		child->writeAscii(stream);
		
		mChildStreams.push_back(std::move(stream));
	}
	
	
	for(std::size_t i = 0; i<m_child_property_names.size(); ++i){
		m_textures[m_child_property_names[i]] = sfbx::as<sfbx::Texture>(m_children[i]);
	}
}

void Material::exportFBXObjects()
{
	super::exportFBXObjects();
	
	for(auto& stream : mChildStreams){
		auto child = getNode()->createChild();
		auto streamString = stream.str();
		auto streamView = std::string_view(streamString);
		child->readAscii(streamView);
	}
}

void Material::exportFBXConnections()
{
	for(auto& parent : getParents()){
		m_document->createLinkOO(shared_from_this(), parent);
	}

	for(std::size_t i = 0; i<m_child_property_names.size(); ++i){
		m_document->createLinkOP(m_children[i], shared_from_this(), m_child_property_names[i]);
	}
}


ObjectClass Implementation::getClass() const { return ObjectClass::Implementation; }

ObjectClass BindingTable::getClass() const { return ObjectClass::BindingTable; }

} // namespace sfbx
