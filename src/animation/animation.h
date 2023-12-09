#ifndef ANIM_ANIMATION_ANIMATION_H
#define ANIM_ANIMATION_ANIMATION_H

#include "bone.h"
// #include "retargeter.h"

#include <string>
#include <map>
#include <filesystem>
#include <iostream>
#include <memory>

namespace sfbx {
class AnimationLayer;
class Document;
}

namespace anim
{

class PoseComponent;
enum AnimationType
{
	None,
	Fbx,
	Json,
	Raw
};

class Animation
{
	friend class MixamoRetargeter;
public:
	Animation() = default;
	Animation(const char *file_path);
	virtual ~Animation() = default;
	Bone *find_bone(const std::string &name);
	float get_fps();
	float get_duration();
	float get_current_duration();
	const std::string &get_name() const;
	const char *get_path() const;
	const std::map<std::string, std::unique_ptr<Bone>> &get_name_bone_map() const;
	std::map<std::string, std::unique_ptr<Bone>> &get_mutable_name_bone_map();
	const AnimationType &get_type() const;
	virtual void reload();
	void get_fbx_animation(std::shared_ptr<sfbx::Document> document, sfbx::AnimationLayer* animationLayer, float factor, bool is_linear);
	void set_id(int id);
	const int get_id() const;
	void add_and_replace_bone(const std::string &name, const glm::mat4 &transform, float time);
	void sub_bone(const std::string &name, float time);
	void replace_bone(const std::string &name, const glm::mat4 &transform, float time);
	
	void set_root_bone_name(const std::string& root_bone_name);
	const std::string& get_root_bone_name() const;

protected:
	float duration_{0.0f};
	int fps_{0};
	std::string name_{};
	std::map<std::string, std::unique_ptr<Bone>> name_bone_map_{};
	std::map<std::string, glm::mat4> name_bindpose_map_{};
	AnimationType type_{};
	std::string path_{};
	int id_{-1};
	
	std::string root_bone_name_;

};

}

#endif
