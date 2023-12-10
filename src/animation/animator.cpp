#include "animator.h"

#include <glm/gtx/matrix_decompose.hpp>
#include "entity.h"
#include "bone.h"
#include "../util/utility.h"
#include "shader.h"
#include "../entity/components/animation_component.h"
#include "../entity/components/renderable/mesh_component.h"
#include "../entity/components/renderable/armature_component.h"
#include "animation.h"
#include "json_animation.h"
#include "assimp_animation.h"

namespace anim
{

Animator::Animator()
: is_stop_(true)
{
	final_bone_matrices_.reserve(MAX_BONE_NUM);
	for (unsigned int i = 0U; i < MAX_BONE_NUM; i++)
		final_bone_matrices_.push_back(glm::mat4(1.0f));
}

void Animator::update(float dt)
{
	if (!is_stop_)
	{
		current_time_ += fps_ * dt * direction_;
		current_time_ = fmax(start_time_, fmod(end_time_ + current_time_, end_time_));
	}
	else
	{
		current_time_ = floor(current_time_);
	}
}

void Animator::update_animation(AnimationComponent *animation, Entity *root, Shader *shader)
{
	assert(animation && root && shader);
	
	
	factor_ = animation->get_ticks_per_second_factor();
	
	if(!animation->get_animation_stack().empty()){
		calculate_bone_transform(root, animation->get_animation_stack(), glm::mat4(1.0f));
	}
	
	calculate_mesh_transform(root->get_mutable_root(), animation->get_animation_stack(), root->get_mutable_root()->get_local());

	shader->use();
	for (int i = 0; i < MAX_BONE_NUM; ++i)
	{
		shader->set_mat4("finalBonesMatrices[" + std::to_string(i) + "]", final_bone_matrices_[i]);
	}
	
	
}

void Animator::calculate_bone_transform(Entity *entity, std::vector<std::shared_ptr<StackedAnimation>>& animationStack, const glm::mat4 &parentTransform)
{
	const std::string &node_name = entity->get_name();
	auto armature = entity->get_component<ArmatureComponent>();
	glm::mat4 global_transformation = parentTransform;
	entity->set_local(glm::mat4(1.0f));
	
	// 바인딩 포즈
	global_transformation *= armature->get_bindpose();
	glm::mat4 blendedLocal = glm::identity<glm::mat4>();
	
	float previousIntersectionTime = 0;
	float previousDuration = 0;
	
	float previousStartTime = 0;
	float previousEndTime = 0;
	
	float previousWrappedTime = 0;
		
	for (auto it = animationStack.begin(); it != animationStack.end(); ++it)
	{
		auto currentAnimation = *it;
		int start_time = currentAnimation->get_start_time();
		int end_time = currentAnimation->get_end_time();
		int duration = currentAnimation->get_duration();
		int wrapped_time = current_time_;
		
		if (wrapped_time > end_time || wrapped_time < start_time) {
			wrapped_time = start_time;
		}

		wrapped_time = std::fmod(wrapped_time - start_time, duration);
		
		if(animationStack.size() == 1){
			
			// Animation
			auto bone = currentAnimation->get_wrapped_animation()->find_bone(node_name);
			if (bone != nullptr && bone->get_time_set().size() > 0)
			{
				auto local = bone->get_local_transform(wrapped_time, factor_);
				if (entity->get_mutable_parent()->get_component<ArmatureComponent>() == nullptr && mIsRootMotion)
				{
					local = glm::mat4(glm::mat3(local));
				}
				
				// Blend with the previous transformation per bone
				
				blendedLocal = local;
			}
		} else {
			
			// Calculate the blend factor based on the intersection
			float blendFactor = 1.0f;
			if(previousDuration != 0){
				
				if(start_time >= previousStartTime && end_time <= previousEndTime){
					// If the previous animation completely covers the current animation
					blendFactor = glm::clamp((current_time_ - previousIntersectionTime) / (previousDuration - previousIntersectionTime), 0.0f, 1.0f);
					
					// Adjust the blendFactor for the second part of the animation (interpolating back to 0)
					float secondPartStartTime = (end_time + start_time) / 2.0f;
					float secondPartDuration = duration / 2;
					
					if (current_time_ >= secondPartStartTime && current_time_ <= end_time) {
						blendFactor = glm::clamp(1.0f - (current_time_ - secondPartStartTime) / (end_time - secondPartStartTime), 0.0f, 1.0f);
					} else {
						blendFactor = glm::clamp((current_time_ - previousIntersectionTime) / (secondPartStartTime - previousIntersectionTime), 0.0f, 1.0f);
					}
				} else {
					blendFactor =  glm::clamp(std::fabs((current_time_ - previousIntersectionTime) / (previousEndTime - previousIntersectionTime)), 0.0f, 1.0f);
				}
			}
			
			// Animation
			auto bone = currentAnimation->get_wrapped_animation()->find_bone(node_name);
			if (bone != nullptr && bone->get_time_set().size() > 0)
			{
				auto local = bone->get_local_transform(wrapped_time, factor_);
				if (entity->get_mutable_parent()->get_component<ArmatureComponent>() == nullptr && mIsRootMotion)
				{
					local = glm::mat4(glm::mat3(local));
				}
				
				for (int i = 0; i < 4; ++i)
				{
					for (int j = 0; j < 4; ++j)
					{
						blendedLocal[i][j] = glm::mix(blendedLocal[i][j], local[i][j], blendFactor);
					}
				}
			}
		}
		
		// Calculate the intersection time with the next animation
		previousIntersectionTime = (it + 1 != animationStack.end()) ? std::min(end_time, (*(it + 1))->get_start_time()) : end_time;

		previousDuration = duration;
		
		previousStartTime = start_time;
		previousEndTime = end_time;
		previousWrappedTime = wrapped_time;
	}
	
	entity->set_local(blendedLocal);
	

	global_transformation *= entity->get_local();

	// FK
	int id = armature->get_id();
	auto &offset = armature->get_bone_offset();
	if (id < MAX_BONE_NUM)
	{
		// 역바인딩변환 행렬과 변환행렬을 곱해줌 (본공간 => 로컬공간)
	
		final_bone_matrices_[id] = global_transformation * offset;
	}
	
	auto &children = entity->get_mutable_children();
	size_t size = children.size();
	armature->set_model_pose(global_transformation);
	for (size_t i = 0; i < size; i++)
	{
		calculate_bone_transform(children[i].get(), animationStack, global_transformation);
	}
}


void Animator::calculate_mesh_transform(Entity *entity, std::vector<std::shared_ptr<StackedAnimation>>& animationStack, const glm::mat4 &parentTransform)
{
	const std::string &node_name = entity->get_name();
	glm::mat4 global_transformation = parentTransform;
	
	glm::mat4 blendedLocal = glm::identity<glm::mat4>();
	
	bool modified = false;
	
	float previousIntersectionTime = 0;
	float previousDuration = 0;
	
	float previousStartTime = 0;
	float previousEndTime = 0;
	
	float previousWrappedTime = 0;
	
	for (auto it = animationStack.begin(); it != animationStack.end(); ++it)
	{
		auto currentAnimation = *it;
		int start_time = currentAnimation->get_start_time();
		int end_time = currentAnimation->get_end_time();
		int duration = currentAnimation->get_duration();
		int wrapped_time = current_time_;
		
		if (wrapped_time > end_time || wrapped_time < start_time) {
			wrapped_time = start_time;
		}
		
		wrapped_time = std::fmod(wrapped_time - start_time, duration);
		
		if(animationStack.size() == 1){
			
			// Animation
			auto bone = currentAnimation->get_wrapped_animation()->find_bone(node_name);
			if (bone != nullptr && bone->get_time_set().size() > 0)
			{
				auto local = bone->get_local_transform(wrapped_time, factor_);
				
				if (mIsRootMotion)
				{
					local = glm::mat4(glm::mat3(local));
				}
				
				// Blend with the previous transformation per bone
				
				blendedLocal = local;
				
				modified = true;
			}
		} else {
			
			// Calculate the blend factor based on the intersection
			float blendFactor = 1.0f;
			if(previousDuration != 0){
				
				if(start_time >= previousStartTime && end_time <= previousEndTime){
					// If the previous animation completely covers the current animation
					blendFactor = glm::clamp((current_time_ - previousIntersectionTime) / (previousDuration - previousIntersectionTime), 0.0f, 1.0f);
					
					// Adjust the blendFactor for the second part of the animation (interpolating back to 0)
					float secondPartStartTime = (end_time + start_time) / 2.0f;
					float secondPartDuration = duration / 2;
					
					if (current_time_ >= secondPartStartTime && current_time_ <= end_time) {
						blendFactor = glm::clamp(1.0f - (current_time_ - secondPartStartTime) / (end_time - secondPartStartTime), 0.0f, 1.0f);
					} else {
						blendFactor = glm::clamp((current_time_ - previousIntersectionTime) / (secondPartStartTime - previousIntersectionTime), 0.0f, 1.0f);
					}
				} else {
					blendFactor =  glm::clamp(std::fabs((current_time_ - previousIntersectionTime) / (previousEndTime - previousIntersectionTime)), 0.0f, 1.0f);
				}
			}
			
			// Animation
			auto bone = currentAnimation->get_wrapped_animation()->find_bone(node_name);
			if (bone != nullptr && bone->get_time_set().size() > 0)
			{
				auto local = bone->get_local_transform(wrapped_time, factor_);
				if (entity->get_mutable_parent()->get_component<ArmatureComponent>() == nullptr && mIsRootMotion)
				{
					local = glm::mat4(glm::mat3(local));
				}
				
				for (int i = 0; i < 4; ++i)
				{
					for (int j = 0; j < 4; ++j)
					{
						blendedLocal[i][j] = glm::mix(blendedLocal[i][j], local[i][j], blendFactor);
					}
				}
				
				modified = true;
			}
		}
		
		// Calculate the intersection time with the next animation
		previousIntersectionTime = (it + 1 != animationStack.end()) ? std::min(end_time, (*(it + 1))->get_start_time()) : end_time;
		
		previousDuration = duration;
		
		previousStartTime = start_time;
		previousEndTime = end_time;
		previousWrappedTime = wrapped_time;
	}
	
	if(modified){
		entity->set_local(blendedLocal);
	}
	
	global_transformation *= entity->get_local();
		
	auto &children = entity->get_mutable_children();
	size_t size = children.size();
	for (size_t i = 0; i < size; i++)
	{
		calculate_mesh_transform(children[i].get(), animationStack, global_transformation);
	}
}

const float Animator::get_current_time() const
{
	return current_time_;
}
const float Animator::get_start_time() const
{
	return start_time_;
}
const float Animator::get_end_time() const
{
	return end_time_;
}
const float Animator::get_fps() const
{
	return fps_;
}
const float Animator::get_direction() const
{
	return direction_;
}
const bool Animator::get_is_stop() const
{
	return is_stop_;
}
void Animator::set_current_time(float current_time)
{
	current_time_ = current_time;
}
void Animator::set_start_time(float time)
{
	start_time_ = time;
}
void Animator::set_end_time(float time)
{
	end_time_ = time;
}
void Animator::set_fps(float fps)
{
	fps_ = fps;
}
void Animator::set_direction(bool is_left)
{
	direction_ = 1.0f;
	if (is_left)
	{
		direction_ = -1.0f;
	}
}
void Animator::set_is_stop(bool is_stop)
{
	is_stop_ = is_stop;
}

}
