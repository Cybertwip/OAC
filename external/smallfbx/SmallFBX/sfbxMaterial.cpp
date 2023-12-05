#include "sfbxInternal.h"
#include "sfbxMaterial.h"
#include "sfbxDocument.h"

namespace sfbx {

ObjectClass Video::getClass() const { return ObjectClass::Video; }
ObjectSubClass Video::getSubClass() const { return ObjectSubClass::Clip; }

void Video::importFBXObjects()
{
	super::importFBXObjects();
	// todo
	
	for(auto& child : getNode()->getChildren()){
		auto stream = std::stringstream();
		
		child->writeAscii(stream);
		
		mChildStreams.push_back(std::move(stream));
	}
}


void Video::exportFBXObjects()
{
	super::exportFBXObjects();
	
	for(auto& stream : mChildStreams){
		auto child = getNode()->createChild();
		auto streamString = stream.str();
		auto streamView = std::string_view(streamString);
		child->readAscii(streamView);
	}
}

ObjectClass Texture::getClass() const { return ObjectClass::Texture; }


void Texture::importFBXObjects()
{
	super::importFBXObjects();
	// todo
	
	for(auto& child : getNode()->getChildren()){
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
	// ignore super::constructLinks()
	m_document->createLinkOO(this, getParent());
	
	for(std::size_t i = 0; i<m_child_property_names.size(); ++i){
		m_document->createLinkOO(m_children[i], this);
		m_document->createLinkOP(m_children[i], this, m_child_property_names[i]);
	}
	
	
	
}


ObjectClass Implementation::getClass() const { return ObjectClass::Implementation; }

ObjectClass BindingTable::getClass() const { return ObjectClass::BindingTable; }

} // namespace sfbx
