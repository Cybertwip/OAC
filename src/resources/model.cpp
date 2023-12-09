#include "model.h"

#include <glad/glad.h>
#include <stb/stb_image.h>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>
#include <filesystem>
#include <queue>

#include "../graphics/shader.h"
#include "../util/utility.h"
#include "../graphics/opengl/gl_mesh.h"
#include "../graphics/mesh.h"

namespace anim
{
    using namespace gl;
    Model::Model(const std::string& path, const sfbx::DocumentPtr doc)
    {
        load_model(path, doc);
    }

    Model::~Model()
    {
        root_node_ = nullptr;
    }

    void Model::load_model(const std::string& path, const sfbx::DocumentPtr doc)
    {
        directory_ = std::filesystem::u8path(path);
        name_ = directory_.filename().string();
        process_node(root_node_, doc->getRootModel(), doc);
        textures_loaded_.clear();
    }

    void Model::process_node(std::shared_ptr<ModelNode> &model_node, const std::shared_ptr<sfbx::Model> node, const sfbx::DocumentPtr doc)
    {
		std::vector<std::shared_ptr<sfbx::Mesh>> meshes;
		std::vector<std::shared_ptr<sfbx::Model>> models;
		std::vector<sfbx::ObjectPtr> filteredMeshes;
		std::vector<sfbx::ObjectPtr> filteredModels;
		
		auto meshCondition = [](const sfbx::ObjectPtr& nodePtr) {
			return std::dynamic_pointer_cast<sfbx::Mesh>(nodePtr) != nullptr;
		};
		auto modelCondition = [](const sfbx::ObjectPtr& nodePtr) {
			return std::dynamic_pointer_cast<sfbx::Model>(nodePtr) != nullptr;
		};

		auto nodes = node->getChildren();
		
		std::copy_if(nodes.begin(), nodes.end(), std::back_inserter(filteredMeshes), meshCondition);
		std::copy_if(nodes.begin(), nodes.end(), std::back_inserter(filteredModels), modelCondition);
		
		for(auto& filteredObject : filteredMeshes){
			meshes.push_back(std::dynamic_pointer_cast<sfbx::Mesh>(filteredObject));
		}
		
		for(auto& filteredObject : filteredModels){
			models.push_back(std::dynamic_pointer_cast<sfbx::Model>(filteredObject));
		}

				
        node_count_++;
        std::string model_name = std::string{node->getName()};
        model_node.reset(new ModelNode(SfbxMatToGlmMat(node->getLocalMatrix()),
			 model_name,
			 filteredModels.size()));
		auto identity = glm::identity<glm::mat4>();
		
//		model_node.reset(new ModelNode(identity,
//			 model_name,
//		   	filteredModels.size()));
		
        for (unsigned int i = 0; i < meshes.size(); i++)
        {
			auto mesh = meshes[i];
            model_node->meshes.emplace_back(process_mesh(mesh, doc));
			
            model_node->has_bone = mesh->getGeometry()->hasSkinDeformer();
        }

        for (unsigned int i = 0; i < filteredModels.size(); i++)
        {
			auto child = sfbx::as<sfbx::Model>(filteredModels[i]);
			
			if(child){
				process_node(model_node->childrens[i], child, doc);

			}
        }
    }

    std::shared_ptr<Mesh> Model::process_mesh(const std::shared_ptr<sfbx::Mesh> mesh, const sfbx::DocumentPtr doc)
    {
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;
        std::vector<Texture> textures;
        MaterialProperties mat_properties;

		auto meshName = std::string{mesh->getName()};
        LOG(meshName);

		auto geometry = mesh->getGeometry();
		
        // process vertex
		auto points = geometry->getPointsDeformed();
		auto normals = geometry->getNormals();
		auto vertexIndices = geometry->getIndices();

        for (unsigned int i = 0; i < points.size(); i++)
        {
			auto& point = points[i];
			
            Vertex vertex;
            vertex.set_position(SfbxVecToGlmVec(point));
            if (!normals.empty())
            {
				auto& normal =
				normals[i];

                vertex.set_normal(SfbxVecToGlmVec(normal));
            }

//            if (mesh->mTangents)
//            {
//                vertex.set_tangent(glm::vec3{
//                    mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z});
//            }
//            if (mesh->mBitangents)
//            {
//                vertex.set_bitangent(glm::vec3{
//                    mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z});
//            }

            vertices.push_back(vertex);
        }
				
		auto uvLayers = geometry->getUVLayers();
		int layerIndex = 0;
		for (auto& uvLayer : uvLayers)
		{
			
			for (int i = 0; i < vertexIndices.size(); i++) {
				int index;
				int uv_index = uvLayer.indices[i];
				index = vertexIndices[i];

				if(layerIndex == 0){
					vertices[index].set_texture_coords1({uvLayer.data[uv_index].x,
						uvLayer.data[uv_index].y});
					vertices[index].set_texture_coords2({uvLayer.data[uv_index].x,
						uvLayer.data[uv_index].y});

				} else {
					vertices[index].set_texture_coords2({uvLayer.data[uv_index].x,
						uvLayer.data[uv_index].y});

				}
			}

			layerIndex++;
			
		}


        process_bone(mesh, doc, vertices);

        // process indices
		
		for(unsigned int i = 0; i<geometry->getIndices().size(); ++i){
			indices.push_back(geometry->getIndices()[i]);
		}
		
		
		if(mesh->getMaterials().size() > 0){
			auto material = mesh->getMaterials()[0];
			
			auto color = material->getAmbientColor();
			
			mat_properties.ambient.x = color.x;
			mat_properties.ambient.y = color.y;
			mat_properties.ambient.z = color.z;

			color = material->getDiffuseColor();
			
			mat_properties.diffuse.x = color.x;
			mat_properties.diffuse.y = color.y;
			mat_properties.diffuse.z = color.z;

			color = material->getSpecularColor();
			
			mat_properties.specular.x = color.x;
			mat_properties.specular.y = color.y;
			mat_properties.specular.z = color.z;

			mat_properties.shininess = material->getShininess();
			
			std::vector<Texture> diffuseMaps = load_material_textures(material, "DiffuseColor");

			textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());

			if (diffuseMaps.size() > 0)
			{
				mat_properties.has_diffuse_texture = true;
			}


		}
		
        // process material
//        aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];
//        aiColor3D color{};
//        std::vector<Texture> diffuseMaps = load_material_textures(material, scene, aiTextureType_DIFFUSE, "texture_diffuse");
//        textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
//        if (diffuseMaps.size() > 0)
//        {
//            mat_properties.has_diffuse_texture = true;
//        }
//
//        std::vector<Texture> specularMaps = load_material_textures(material, scene, aiTextureType_SPECULAR, "texture_specular");
//        textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
//
//        std::vector<Texture> normalMaps = load_material_textures(material, scene, aiTextureType_HEIGHT, "texture_normal");
//        textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
//
//        std::vector<Texture> heightMaps = load_material_textures(material, scene, aiTextureType_AMBIENT, "texture_height");
//        textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());

		return std::make_shared<GLMesh>(vertices, indices, textures, mat_properties);
    }

    void Model::process_bone(const std::shared_ptr<sfbx::Mesh> mesh, const sfbx::DocumentPtr doc, std::vector<Vertex> &vertices)
    {
        auto &bone_info_map = bone_info_map_;
        int &bone_count = bone_count_;
		
		auto deformers = mesh->getGeometry()->getDeformers();
		
		std::shared_ptr<sfbx::Skin> skin;
		
		for(auto& deformer : deformers){
			if(skin = sfbx::as<sfbx::Skin>(deformer); skin){
				break;
			}
		}
		
		if(skin){
			auto skinClusters = skin->getChildren();
			int bone_num = skinClusters.size();
			for (int bone_idx = 0; bone_idx < bone_num; ++bone_idx)
			{
				auto skinCluster = sfbx::as<sfbx::Cluster>(skinClusters[bone_idx]);
				
				int bone_id = -1;
				std::string bone_name = std::string{skinCluster->getChild()->getName()};
				auto bone_it = bone_info_map.find(bone_name);
				
				auto limbNode = sfbx::as<sfbx::LimbNode>(skinCluster->getChild());

				if (bone_it == bone_info_map.end())
				{
					BoneInfo new_bone_info{};
					new_bone_info.id = bone_count;
					
					new_bone_info.offset = SfbxMatToGlmMat(skinCluster->getTransform());

					bone_info_map[bone_name] = new_bone_info;
					
					bone_id = new_bone_info.id;
					bone_count++;
					LOG("- - bone_name: " + bone_name + " bone_id: " + std::to_string(bone_id));
				}
				else
				{
					bone_id = bone_it->second.id;
				}
				
				assert(bone_id != -1);
				
				auto weights = skinCluster->getWeights();
				auto indices = skinCluster->getIndices();
				
				int weights_num = weights.size();
				
				for (int weight_idx = 0; weight_idx < weights_num; ++weight_idx)
				{
					int vertex_id = indices[weight_idx];
					float weight = weights[weight_idx];
					
					assert(static_cast<size_t>(vertex_id) <= vertices.size());
					vertices[vertex_id].set_bone(bone_id, weight);
				}
			}
		}
		
    }

    std::vector<Texture> Model::load_material_textures(std::shared_ptr<sfbx::Material> mat,   std::string typeName)
    {
//        LOG("LoadMaterialTextures: " + std::to_string((int)type) + " " + std::to_string(mat->GetTextureCount(type)));
        std::vector<Texture> textures;
//        for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
		if(auto materialTexture = mat->getTexture(typeName); materialTexture)
        {
            bool skip = false;
            for (unsigned int j = 0; j < textures_loaded_.size(); j++)
            {
                if (textures_loaded_[j].path.compare( materialTexture->getFilename()) == 0)
                {
                    textures.push_back(textures_loaded_[j]);
                    skip = true;
                    break;
                }
            }
            if (!skip)
            { // if texture hasn't been loaded already, load it
                Texture texture;
                texture.id = TextureFromFile(materialTexture->getFilename(), directory_, materialTexture);
                texture.type = typeName;
                texture.path = std::string{materialTexture->getFilename()};
                textures.push_back(texture);
                textures_loaded_.push_back(texture); // add to loaded textures
            }
        }
        return textures;
    }

    unsigned int TextureFromFile(const std::string_view path, const std::filesystem::path &directory, const std::shared_ptr<sfbx::Texture> materialTexture)
    {
		std::string filename = std::string{path};

        LOG("TextureFromFile:: " + filename);

        size_t idx = filename.find_first_of("/\\");
        if (filename[0] == '.' || idx == 0)
        {
            filename = filename.substr(idx + 1);
        }
        std::filesystem::path tmp = directory;
        filename = tmp.replace_filename(filename).string();
        std::replace(filename.begin(), filename.end(), '\\', '/');

        unsigned int textureID;
        glGenTextures(1, &textureID);

        int width, height, nrComponents;
        unsigned char *data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
        if (data)
        {
            GLenum format = GL_RGB;
            if (nrComponents == 1)
                format = GL_RED;
            else if (nrComponents == 3)
                format = GL_RGB;
            else if (nrComponents == 4)
                format = GL_RGBA;

            glBindTexture(GL_TEXTURE_2D, textureID);
            glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        }
        else
        {
            if (materialTexture->getEmbedded())
            {
                LoadMemory(materialTexture, &textureID);
            }
            else
            {
                LOG("- - Texture failed to load at path: " + std::string(path));
            }
        }
        stbi_image_free(data);

        return textureID;
    }

    void LoadMemory(const std::shared_ptr<sfbx::Texture> texture, unsigned int *id)
    {
        LOG("LoadMemory");
        if (!*id)
            glGenTextures(1, id);
        glBindTexture(GL_TEXTURE_2D, *id);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        unsigned char *image_data = nullptr;
        int width = 0, height = 0, components_per_pixel;
        
		image_data = stbi_load_from_memory(texture->getData().data(),
			 texture->getData().size(),
			 &width,
			 &height,
			 &components_per_pixel,
			 0);
        

        if (components_per_pixel == 3)
        {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image_data);
        }
        else if (components_per_pixel == 4)
        {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
        }

        glGenerateMipmap(GL_TEXTURE_2D);

        glBindTexture(GL_TEXTURE_2D, 0);
        stbi_image_free(image_data);
    }
}
namespace anim
{
    std::unordered_map<std::string, BoneInfo> &Model::get_mutable_bone_info_map()
    {
        return bone_info_map_;
    }

    const std::unordered_map<std::string, BoneInfo> &Model::get_bone_info_map() const
    {
        return bone_info_map_;
    }

    const BoneInfo *Model::get_pointer_bone_info(const std::string &bone_name) const
    {
        auto it = bone_info_map_.find(bone_name);
        if (it != bone_info_map_.end())
        {
            return &it->second;
        }
        return nullptr;
    }

    int &Model::get_mutable_bone_count()
    {
        return bone_count_;
    }

    const std::shared_ptr<ModelNode> &Model::get_root_node() const
    {
        return root_node_;
    }
    std::shared_ptr<ModelNode> &Model::get_mutable_root_node()
    {
        return root_node_;
    }
    const std::string &Model::get_name() const
    {
        return name_;
    }

	void Model::set_unit_scale(float scale){
		unit_scale_ = scale;
	}

	const float Model::get_unit_scale()	const{
		return unit_scale_;
	}
}
