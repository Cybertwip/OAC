#include "component_layer.h"

#include "scene/scene.hpp"
#include "imgui_helper.h"
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <entity/entity.h>
#include <entity/components/renderable/mesh_component.h>
#include <entity/components/animation_component.h>
#include <animation/animation.h>

using namespace anim;

namespace ui
{
    ComponentLayer::ComponentLayer() = default;

    ComponentLayer::~ComponentLayer() = default;

    void ComponentLayer::draw(ComponentContext &context, Scene *scene)
    {
        Entity *entity = scene->get_mutable_selected_entity();
        SharedResources *resources = scene->get_mutable_ref_shared_resources().get();

		ImGuiWindowClass window_class;
		window_class.DockNodeFlagsOverrideSet = ImGuiDockNodeFlags_NoTabBar;
		
		ImGui::SetNextWindowClass(&window_class);

        if (ImGui::Begin("Component", nullptr, ImGuiWindowFlags_NoMove))
        {
            if (entity)
            {
                if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_Bullet))
                {
                    draw_transform(entity);
                    ImGui::Separator();
                }
                if (auto root = entity->get_mutable_root(); root)
                {
                    if (auto animation = root->get_component<AnimationComponent>(); animation && ImGui::CollapsingHeader("Animation", ImGuiTreeNodeFlags_Bullet))
                    {
                        draw_animation(context, resources, root, animation);
                        ImGui::Separator();
                    }
                }
                if (auto mesh = entity->get_component<anim::MeshComponent>(); mesh && ImGui::CollapsingHeader("Mesh", ImGuiTreeNodeFlags_Bullet))
                {
                    draw_mesh(mesh);
                }
            }
        }
        ImGui::End();
    }

    void ComponentLayer::draw_animation(ComponentContext &context, SharedResources *shared_resource, Entity *entity, const AnimationComponent *animation)
    {
		if(animation->get_animation() != nullptr){
			context.current_animation_idx = animation->get_animation()->get_id();
		} else {
			context.current_animation_idx = -1;
			context.is_changed_animation = true;
		}
        int animation_idx = context.current_animation_idx;

        auto &resourceAnimations = shared_resource->get_animations();
		
		std::vector<std::shared_ptr<Animation>> animations;
		
		for(auto& resourceAnimation : resourceAnimations){
			if(resourceAnimation->get_owner() == entity->get_mutable_root()){
				animations.push_back(resourceAnimation);
			}
		}
		
        const char *names[] = {"Animation"};
        ImGuiStyle &style = ImGui::GetStyle();

        float child_w = (ImGui::GetContentRegionAvail().x - 1 * style.ItemSpacing.x);
        if (child_w < 1.0f)
            child_w = 1.0f;

        ImGui::PushID("##VerticalScrolling");
        for (int i = 0; i < 1; i++)
        {
            const ImGuiWindowFlags child_flags = 0;
            const ImGuiID child_id = ImGui::GetID((void *)(intptr_t)i);
            const bool child_is_visible = ImGui::BeginChild(child_id, ImVec2(child_w, 200.0f), true, child_flags);
//            if (ImGui::BeginMenuBar())
//            {
//                ImGui::TextUnformatted(names[i]);
//                ImGui::EndMenuBar();
//            }
            if (child_is_visible)
            {
                for (int idx = 0; idx < animations.size(); idx++)
                {
                    std::string name = std::to_string(idx) + ":" + animations[idx]->get_name();

                    bool is_selected = (animations[idx]->get_id() == animation_idx);
                    if (ImGui::Selectable(name.c_str(), is_selected))
                    {
                        animation_idx = animations[idx]->get_id();
                    }
					
					if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
					{
						context.is_add_animation_track = true;
					}
                }
            }

            ImGui::EndChild();
        }
        if(ImGui::Button("+")) {
            context.is_clicked_retargeting = true;
        }
		
		ImGui::SameLine();
		if(ImGui::Button("-")) {
			context.is_clicked_remove_animation = true;
		}

        auto animc = const_cast<AnimationComponent *>(animation);
        auto anim = animc->get_mutable_animation();
//        ImGui::Text("duration: %f", anim->get_duration());
//        ImGui::Text("fps: %f", anim->get_fps());
        float &fps = animc->get_mutable_custom_tick_per_second();
        if (animation_idx != context.current_animation_idx)
        {
            context.new_animation_idx = animation_idx;
            context.is_changed_animation = true;
        }
//        ImGui::DragFloat("custom fps", &fps, 1.0f, 1.0f, 144.0f);
        ImGui::PopID();
    }

    void ComponentLayer::draw_transform(anim::Entity *entity)
    {
        auto &world = entity->get_world_transformation();
        auto &local = entity->get_local();
        TransformComponent transform;
        transform.set_transform(world);
        ImGui::Text("World");
        DragPropertyXYZ("Translation", transform.mTranslation);
        DragPropertyXYZ("Rotation", transform.mRotation);
        DragPropertyXYZ("Scale", transform.mScale);

        ImGui::Separator();

        transform.set_transform(local);
        ImGui::Text("Local");
        DragPropertyXYZ("Translation", transform.mTranslation);
        DragPropertyXYZ("Rotation", transform.mRotation);
        DragPropertyXYZ("Scale", transform.mScale);
    }
    void ComponentLayer::draw_transform_reset_button(anim::TransformComponent &transform)
    {
        if (ImGui::Button("reset"))
        {
            transform.set_translation({0.0f, 0.0f, 0.0f}).set_rotation({0.0f, 0.0f, 0.0f}).set_scale({1.0f, 1.0f, 1.0f});
        }
    }
    void ComponentLayer::draw_mesh(anim::MeshComponent *mesh)
    {
        auto material = mesh->get_mutable_mat();
        int idx = 0;
        for (auto &mat : material)
        {
            ImGui::ColorPicker3(("diffuse " + std::to_string(idx)).c_str(), &mat->diffuse[0]);
			
			idx++;
        }
    }
}
