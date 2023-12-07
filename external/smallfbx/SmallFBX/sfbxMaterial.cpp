#include "sfbxInternal.h"
#include "sfbxMaterial.h"
#include "sfbxDocument.h"

namespace sfbx {

ObjectClass Video::getClass() const { return ObjectClass::Video; }
ObjectSubClass Video::getSubClass() const { return ObjectSubClass::Clip; }

bool Video::getEmbedded() const {
	return m_embedded;
}

void Video::importFBXObjects()
{
	super::importFBXObjects();
	// todo

	std::string embeddedFilename;

	bool isEmbedded = false;
	
	for(auto& child : getNode()->getChildren()){

		if(child->getName() == "Filename"){
			embeddedFilename = child->getProperty(0)->getString();
		}
		
		if(child->getName() == "RelativeFilename"){
			embeddedFilename = child->getProperty(0)->getString();
		}
		
		if(child->getName() == "Content"){
			isEmbedded = true;
		}

	}
	
	
	if(isEmbedded){
		embeddedFilename = "*:" + std::to_string(this->getID());
	}
	
	for(auto& child : getNode()->getChildren()){
		
		if(child->getName() == "Properties70"){
			continue;
		}

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


void Texture::importFBXObjects()
{
	super::importFBXObjects();
	// todo
	
	auto video = find_if(m_children, [](ObjectPtr ptr){
		return sfbx::as<Video>(ptr);
	});
	
	bool hasVideo = false;
	
	std::string embeddedFilename;
	
	if(video){
		hasVideo = true;
		
		embeddedFilename = "*:" + std::to_string(video->getID());

	}
	
	
	for(auto& child : getNode()->getChildren()){

		if(child->getName() == "Properties70"){
			continue;
		}
		
		if(child->getName() == "FileName" || child->getName() == "RelativeFilename"){
			
			if(hasVideo){
				if(sfbx::as<Video>(video)->getEmbedded()){
					child->getProperty(0)->assign(embeddedFilename);
				}
			}
		}

		auto stream = std::stringstream();
		
		child->writeAscii(stream);
		
		mChildStreams.push_back(std::move(stream));
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

void Material::importFBXObjects()
{
    super::importFBXObjects();
    // todo
	
	for(auto& child : getNode()->getChildren()){
		auto stream = std::stringstream();
		
		child->writeAscii(stream);
		
		mChildStreams.push_back(std::move(stream));
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
