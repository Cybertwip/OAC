#include "sfbxInternal.h"
#include "sfbxMaterial.h"
#include "sfbxDocument.h"

namespace sfbx {

ObjectClass Video::getClass() const { return ObjectClass::Video; }
ObjectSubClass Video::getSubClass() const { return ObjectSubClass::Clip; }

void Video::importFBXObjects()
{
	for(auto& child : getNode()->getChildren()){
		auto stream = std::stringstream();
		
		child->writeBinary(stream, 0);
		
		mChildStreams.push_back(std::move(stream));
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


void Video::exportFBXConnections()
{
}


ObjectClass Texture::getClass() const { return ObjectClass::Texture; }


void Texture::importFBXObjects()
{
	for(auto& child : getNode()->getChildren()){
		auto stream = std::stringstream();
		
		child->writeBinary(stream, 0);
		
		mChildStreams.push_back(std::move(stream));
	}
	
}

void Texture::exportFBXObjects()
{
	super::exportFBXObjects();
	
	for(auto& stream : mChildStreams){
		auto child = getNode()->createChild();
		
		stream.seekg(std::ios::beg);
		
		child->readBinary(stream, 0);
	}
}

void Texture::exportFBXConnections()
{
	for(auto& child : m_children){
		m_document->createLinkOO(child, shared_from_this());
	}
}
ObjectClass Material::getClass() const { return ObjectClass::Material; }

void Material::importFBXObjects()
{
	
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
		m_document->createLinkOO(m_children[i], shared_from_this());
		m_document->createLinkOP(m_children[i], shared_from_this(), m_child_property_names[i]);
	}
}


ObjectClass Implementation::getClass() const { return ObjectClass::Implementation; }

ObjectClass BindingTable::getClass() const { return ObjectClass::BindingTable; }

} // namespace sfbx
