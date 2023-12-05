#include "timeline_layer.h"
#include "text_edit_layer.h"
#include "scene/scene.hpp"

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <imgui/imgui_neo_sequencer.h>
#include <imgui/icons/icons.h>
#include <imgui/ImGuizmo.h>
#include <imgui/ImSequencer.h>
#include <imgui/ImCurveEdit.h>

#include <entity/entity.h>
#include <resources/shared_resources.h>
#include <animation/animation.h>
#include <animation/animator.h>
#include <animation/bone.h>
#include <entity/components/pose_component.h>
#include <entity/components/animation_component.h>
#include <glm/gtc/type_ptr.hpp>

#include "imgui_helper.h"

#include <iostream>


namespace {

static const char* SequencerItemTypeNames[] = { "Animation" };


struct TracksSequencer : public ImSequencer::SequenceInterface
{
	// interface with sequencer
	
	virtual int GetFrameMin() const {
		return 0;
//		return mFrameMin;
	}
	virtual int GetFrameMax() const {
		return mFrameMax;
	}
	virtual int GetItemCount() const { return (int)mItems.size(); }
	
	virtual int GetItemTypeCount() const { return sizeof(SequencerItemTypeNames) / sizeof(char*); }
	virtual const char* GetItemTypeName(int typeIndex) const { return SequencerItemTypeNames[typeIndex]; }
	virtual const char* GetItemLabel(int index) const
	{
		static char tmps[512];
		snprintf(tmps, 512, "[%02d] %s", index, mItems[index].mName.c_str());
		return tmps;
	}
	
	virtual void Get(int index, int** start, int** end, int* type, unsigned int* color)
	{
		SequenceItem& item = mItems[index];
		if (color)
			*color = 0xFFAA8080; // same color for everyone, return color based on type
		
		if(item.mFrameStart == item.mFrameEnd){
			item.mFrameEnd = item.mFrameStart + 1;
		}
		
		if (start)
			*start = &item.mFrameStart;
		if (end)
			*end = &item.mFrameEnd;
		if (type)
			*type = item.mType;
	}
	virtual void Add(int type) { mItems.push_back(SequenceItem(-1, "", type, 0, 10 )); };
	virtual void Del(int index) { mItems.erase(mItems.begin() + index); }
	virtual void Duplicate(int index) { mItems.push_back(mItems[index]); }
	
	virtual size_t GetCustomHeight(int index) { return 0; }
	
	virtual void CustomDrawCompact(int index, ImDrawList* draw_list, const ImRect& rc, const ImRect& clippingRect)
	{
		draw_list->PushClipRect(clippingRect.Min, clippingRect.Max, true);
		
		int standardSize = mItems[index].mSequenceSize;
		int extendedSize = mItems[index].mFrameEnd - mItems[index].mFrameStart;
		if(extendedSize > standardSize){
			int numberOfFits = static_cast<int>(extendedSize / standardSize);

			
			for(int fit = 0; fit < numberOfFits; ++fit){
				
				int repeatingFactor = fit + 1;
				
				float r = standardSize / float(mFrameMax);
				
				float x = ImLerp(rc.Min.x, rc.Max.x, r * repeatingFactor + r * mItems[index].mFrameStart / float(mItems[index].mFrameTotal));
								
				draw_list->AddLine(ImVec2(x, rc.Min.y + 6), ImVec2(x, rc.Max.y - 4), 0xAA000000, 4.f);
			}

		}
		
		draw_list->PopClipRect();
	}
	
	TracksSequencer() : 
//	mFrameMin(0),
	mFrameMax(0) {
		
//		mFrameMin = 0;
		mFrameMax = 1200;

	}
//	int mFrameMin;
	int mFrameMax;
	struct SequenceItem
	{
		SequenceItem(int id, const std::string& name, int type, int frameStart, int frameTotal){
			mId = id;
			mName = name;
			mType = type;
			mFrameStart = frameStart;
			mFrameTotal = frameTotal;
			
			mFrameEnd = mFrameTotal;
			mSequenceSize = mFrameTotal - mFrameStart;
		}
		
		int mId;
		std::string mName;
		int mType;
		int mFrameStart, mSequenceSize, mFrameTotal, mFrameEnd;
	};
	std::vector<SequenceItem> mItems;
	
};


static TracksSequencer g_tracksSequencer;

}


using namespace anim;

namespace ui
{

    TimelineLayer::TimelineLayer()
    {
        text_editor_.reset(new TextEditLayer());
        text_editor_->init();
	}

	void TimelineLayer::init(Scene *scene){
		resources_ = scene->get_mutable_shared_resources().get();

		const auto &animations = resources_->get_animations();
		
		if(!animations.empty()){
			g_tracksSequencer.mItems.push_back(TracksSequencer::SequenceItem(animations[0]->get_id(), animations[0]->get_name(), 0, 0, animations[0]->get_duration()));
		}
	}

    void TimelineLayer::draw(Scene *scene, UiContext &ui_context)
    {
        init_context(ui_context, scene);
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_AlwaysAutoResize;
		
		if(ui_context.component.is_add_animation_track){
			ui_context.component.is_add_animation_track = false;
			
			const auto &animations = resources_->get_animations();

			const auto& animation = animations[ui_context.component.current_animation_idx];
			
			bool existing = false;
			for(auto& item : g_tracksSequencer.mItems){
				if(item.mId == ui_context.component.current_animation_idx){
					existing = true;
					break;
				}
			}
			
			if(!existing){
				g_tracksSequencer.mItems.push_back({ui_context.component.current_animation_idx, animation->get_name(), 0, 0, (int)animation->get_duration()});
			}
			
		}

        if (is_hovered_zoom_slider_)
        {
            window_flags |= ImGuiWindowFlags_NoScrollWithMouse;
        }


        ImGui::Begin(ICON_MD_SCHEDULE " Playback", 0, window_flags);
        {
 
			ImGui::BeginTabBar("MyTabs", ImGuiTabBarFlags_None);
			
			
			AnimationComponent* anim_component = nullptr;
			
			if(root_entity_){
				anim_component = root_entity_->get_component<AnimationComponent>();

				if (anim_component)
				{
					anim_component->clear_animation_stack();
				}

			}
	
			// Tab: Tracks
			if (ImGui::BeginTabItem(ICON_MD_TRACK_CHANGES " Tracks"))
			{
				if(anim_component){
					const auto &animations = resources_->get_animations();
					
					for(auto& item : g_tracksSequencer.mItems){
						const auto& animation = animations[item.mId];
						
						if(animator_->get_current_time() >= item.mFrameStart && animator_->get_current_time() <= item.mFrameEnd){
							anim_component->stack_animation(std::make_shared<StackedAnimation>(animation, item.mFrameStart, item.mFrameEnd));
							
						}
						
						auto& animationStack = anim_component->get_animation_stack();
						
						// Custom comparator function to sort based on start_time
						auto comparator = [](const std::shared_ptr<StackedAnimation>& a, const std::shared_ptr<StackedAnimation>& b) {
							return a->get_start_time() < b->get_start_time();
						};
						
						// Sort animationStack using the custom comparator
						std::sort(animationStack.begin(), animationStack.end(), comparator);
						
						
					}
					
				}
				
				auto &context = ui_context.timeline;
				ImGuiIO &io = ImGui::GetIO();
				
				ImVec2 button_size(40.0f, 0.0f);
				ImVec2 small_button_size(32.0f, 0.0f);
				const float item_spacing = ImGui::GetStyle().ItemSpacing.x;
				float width = ImGui::GetContentRegionAvail().x;
				
				ImGui::PushItemWidth(30.0f);
				DragFloatProperty(ICON_MD_SPEED, context.fps, 1.0f, 1.0f, 300.0f);
				
				ImGui::SameLine();
				
				auto current_cursor = ImGui::GetCursorPosX();
				auto next_pos = ImGui::GetWindowWidth() / 2.0f - button_size.x - small_button_size.x - item_spacing / 2.0;
				if (next_pos < current_cursor)
				{
					next_pos = current_cursor;
				}
				ImGui::SameLine(next_pos);
				
				ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 2.0f);
				ImGui::PushStyleColor(ImGuiCol_Text, {1.0f, 0.3f, 0.2f, 0.8f});
				{
					ImGui::PopStyleColor();
					ImGui::PushStyleColor(ImGuiCol_Text, {1.0f, 1.0f, 1.0f, 1.0f});
					ToggleButton(ICON_KI_CARET_LEFT, &context.is_backward, small_button_size, &context.is_clicked_play_back);
					ImGui::SameLine();
					ToggleButton(ICON_KI_CARET_RIGHT, &context.is_forward, small_button_size, &context.is_clicked_play);
					ImGui::SameLine();
					ToggleButton(ICON_KI_PAUSE, &context.is_stop, small_button_size);
				}
				ImGui::PopStyleColor();
				ImGui::PopStyleVar();


				// let's create the sequencer
				static int selectedEntry = -1;
				static int firstFrame = 0;
				static bool expanded = true;
				static int currentFrame = 0;
				
				currentFrame = static_cast<int>(animator_->get_current_time());
				
				animator_->set_end_time(g_tracksSequencer.mFrameMax);
				
				button_size = ImVec2(180.0f, 0.0f);

				current_cursor = ImGui::GetCursorPosX();
				next_pos = ImGui::GetWindowWidth() - button_size.x - small_button_size.x - item_spacing / 2.0;
				if (next_pos < current_cursor)
				{
					next_pos = current_cursor;
				}

				ImGui::PushItemWidth(130);
//				ImGui::InputInt("Frame Min", &g_tracksSequencer.mFrameMin);
				ImGui::InputInt("Frame ", &currentFrame);
				ImGui::SameLine(next_pos);
				ImGui::InputInt("Frame Max", &g_tracksSequencer.mFrameMax);
				ImGui::PopItemWidth();
				Sequencer(&g_tracksSequencer, &currentFrame, &expanded, &selectedEntry, &firstFrame, ImSequencer::SEQUENCER_EDIT_STARTEND | ImSequencer::SEQUENCER_ADD | ImSequencer::SEQUENCER_DEL | ImSequencer::SEQUENCER_COPYPASTE | ImSequencer::SEQUENCER_CHANGE_FRAME);
				// add a UI to edit that particular item
				if (selectedEntry != -1)
				{
					const TracksSequencer::SequenceItem &item = g_tracksSequencer.mItems[selectedEntry];
//					ImGui::Text("I am a %s, please edit me", SequencerItemTypeNames[item.mType]);
					// switch (type) ....
				}
				
				ImGui::EndTabItem();
			}
			
			// Tab: Animation
			if (ImGui::BeginTabItem(ICON_MD_SCHEDULE " Animation"))
			{
				const auto &animations = resources_->get_animations();

				if (root_entity_ && root_entity_->get_component<AnimationComponent>())
				{
					if (anim_component && anim_component->get_animation())
					{
						const auto& animation = animations[anim_component->get_animation()->get_id()];
						
						anim_component->stack_animation(std::make_shared<StackedAnimation>(animation, 0, anim_component->get_mutable_animation()->get_duration()));

						animator_->set_end_time(static_cast<float>(anim_component->get_mutable_animation()->get_duration()));
						
						ui_context.timeline.end_frame = static_cast<int>(animator_->get_end_time());
					}
				}

				draw_animator_status(ui_context);
				ImGui::BeginChild("##Timeline", ImVec2(0, 0), false, window_flags);
				{
					draw_sequencer(ui_context);
				}
				ImGui::EndChild();
				
				ImGui::EndTabItem();
			}
			
			
			ImGui::EndTabBar();
		}
        ImGui::End();
		

    }
    inline void TimelineLayer::init_context(UiContext &ui_context, Scene *scene)
    {
        auto &context = ui_context.timeline;
        scene_ = scene;
        entity_ = scene->get_mutable_selected_entity();
        root_entity_ = nullptr;
        if (entity_)
        {
            root_entity_ = entity_->get_mutable_root();
        }
        resources_ = scene->get_mutable_shared_resources().get();
        animator_ = resources_->get_mutable_animator();
        context.fps = animator_->get_fps();
        context.start_frame = static_cast<int>(animator_->get_start_time());
		
		if(ui_context.component.current_animation_idx != -1){
			const auto &animations = resources_->get_animations();
			
			const auto& animation = animations[ui_context.component.current_animation_idx];
			
			context.end_frame = static_cast<int>(animation->get_duration());
		}
		
        context.current_frame = static_cast<int>(animator_->get_current_time());
        context.is_recording = is_recording_;
        if (!context.is_stop)
        {
            context.is_stop = animator_->get_is_stop();
        }
        if (!context.is_stop)
        {
            context.is_forward = (animator_->get_direction() > 0.0f) ? true : false;
            context.is_backward = !context.is_forward;
        }
    }

    void TimelineLayer::draw_animator_status(UiContext &ui_context)
    {
        auto &context = ui_context.timeline;
        ImGuiIO &io = ImGui::GetIO();

        ImVec2 button_size(40.0f, 0.0f);
        ImVec2 small_button_size(32.0f, 0.0f);
        const float item_spacing = ImGui::GetStyle().ItemSpacing.x;
        float width = ImGui::GetContentRegionAvail().x;

        ImGui::PushItemWidth(30.0f);
        DragFloatProperty(ICON_MD_SPEED, context.fps, 1.0f, 1.0f, 300.0f);

        ImGui::SameLine();

        auto current_cursor = ImGui::GetCursorPosX();
        auto next_pos = ImGui::GetWindowWidth() / 2.0f - button_size.x - small_button_size.x - item_spacing / 2.0;
        if (next_pos < current_cursor)
        {
            next_pos = current_cursor;
        }
        ImGui::SameLine(next_pos);

        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 2.0f);
        ImGui::PushStyleColor(ImGuiCol_Text, {1.0f, 0.3f, 0.2f, 0.8f});
        {
            ToggleButton(ICON_KI_REC, &context.is_recording, small_button_size);
            is_recording_ = context.is_recording;
            ImGui::PopStyleColor();
            ImGui::SameLine();
            ImGui::PushStyleColor(ImGuiCol_Text, {1.0f, 1.0f, 1.0f, 1.0f});
            ToggleButton(ICON_KI_CARET_LEFT, &context.is_backward, small_button_size, &context.is_clicked_play_back);
            ImGui::SameLine();
            ToggleButton(ICON_KI_CARET_RIGHT, &context.is_forward, small_button_size, &context.is_clicked_play);
            ImGui::SameLine();
            ToggleButton(ICON_KI_PAUSE, &context.is_stop, small_button_size);
            ImGui::SameLine();
        }
        ImGui::PopStyleColor();
        ImGui::PopStyleVar();

        current_cursor = ImGui::GetCursorPosX();
        auto font_size = ImGui::CalcTextSize("Current Start End").x;
        next_pos = width - font_size + (-30.0f - item_spacing) * 3;

        if (next_pos < current_cursor)
        {
            next_pos = current_cursor;
        }
        ImGui::SameLine(next_pos);
        context.is_current_frame_changed = DragIntProperty("Current", context.current_frame, 1.0f, context.start_frame, context.end_frame);
        ImGui::SameLine();
        DragIntProperty("Start", context.start_frame, 1.0f, 0, context.end_frame - 1);
        ImGui::SameLine();
        DragIntProperty("End", context.end_frame, 1.0f, context.start_frame + 1, 10000);
        ImGui::PopItemWidth();
    }

    void TimelineLayer::draw_sequencer(UiContext &ui_context)
    {
        auto &context = ui_context.timeline;
        uint32_t current = static_cast<uint32_t>(animator_->get_current_time());
        uint32_t start = static_cast<uint32_t>(animator_->get_start_time());
        uint32_t end = static_cast<uint32_t>(animator_->get_end_time());
        uint32_t before = current;
        auto win_pos = ImGui::GetWindowPos();
        auto viewportPanelSize = ImGui::GetContentRegionAvail();
        win_pos.y = ImGui::GetCursorPosY();

        if (ImGui::BeginNeoSequencer("Sequencer", &current, &start, &end))
        {

            if (root_entity_ && root_entity_->get_component<AnimationComponent>())
            {
                auto anim_component = root_entity_->get_component<AnimationComponent>();
                if (anim_component && anim_component->get_animation())
                {
                    draw_keyframes(ui_context, anim_component->get_animation());
                }
            }
            is_hovered_zoom_slider_ = false;
            if (ImGui::IsZoomSliderHovered())
            {
                is_hovered_zoom_slider_ = true;
            }
            
            ImGui::EndNeoSequencer();
        }

        // update current time
        if (before != current)
        {
            context.is_current_frame_changed = true;
            context.current_frame = static_cast<int>(current);
        }
    }

    //TODO: Refactor drag logic
    void TimelineLayer::draw_keyframes(UiContext &ui_context, const Animation *animation)
    {
        static ImRect border{};
        static float mouse_wheel = 0.0f;
        static int selected_frame_count = 0;

        auto &context = ui_context.timeline;
        auto &entity_context = ui_context.entity;

        // for select keyframe
        ImGuiWindow *window = ImGui::GetCurrentWindow();
        ImGuiIO &io = ImGui::GetIO();
        bool is_released = false;
        auto start_pos = io.MouseClickedPos[0];
        bool is_okay_start_drag = false;
        if(window->InnerRect.Contains(start_pos) &&!ImGui::IsCurrentFrameHovered() && !ImGui::IsZoomSliderHovered()) {
            is_okay_start_drag = true;
        }
        if(ImGui::IsMouseHoveringRect(window->InnerRect.Min, window->InnerRect.Max)) {
            if (is_okay_start_drag && ImGui::IsMouseDragging(ImGuiMouseButton_Left))
            {
                ImGui::GetForegroundDrawList()->AddRectFilled(start_pos,
                                                              io.MousePos,
                                                              ImGui::GetColorU32(ImGuiCol_Button)); // Draw a line between the button and the mouse cursor
                border.Min.x = std::min(start_pos.x, io.MousePos.x);
                border.Max.x = std::max(start_pos.x, io.MousePos.x);
                border.Min.y = std::min(start_pos.y, io.MousePos.y);
                border.Max.y = std::max(start_pos.y, io.MousePos.y);
            }
            else {
                is_released = true;
            }
            if(is_released && ImGui::IsMouseClicked(ImGuiMouseButton_Right)&& !ImGui::IsPopupOpen("keyframe_popup")) {
                ImGui::OpenPopup("drag_popup");
            }
        }
        
        bool is_selected_keyframe_delete = draw_drag_popup();
        if(!ImGui::IsPopupOpen("drag_popup")&&(ImGui::IsMouseClicked(ImGuiMouseButton_Left) || mouse_wheel != io.MouseWheel ||ImGui::IsMouseClicked(ImGuiMouseButton_Middle))) 
        {
            border = ImRect{};
        }
        mouse_wheel = io.MouseWheel;
        
        // draw transform keyframes
        if (ImGui::BeginNeoGroup("Transform", &is_opened_transform_))
        {
            auto &name_bone_map = animation->get_name_bone_map();
            std::vector<std::pair<Bone*, float>> selected_bone_list;
            for (auto &bone : name_bone_map)
            {
                float factor = bone.second->get_factor();
                auto &keys = bone.second->get_time_set();
                const char *name = bone.second->get_name().c_str();

                if (ImGui::BeginNeoTimeline(name))
                {
                    int inside_count = 0;
                    for (auto key : keys)
                    {
                        bool is_hovered = false;
                        uint32_t ukey = static_cast<uint32_t>(floorf(key * factor));
                        bool change_selected_entity = false;
                        bool is_inside_border = false;
                        if (ImGui::Keyframe(&ukey, border, &is_inside_border, &is_hovered) && is_hovered && ImGui::IsItemClicked())
                        {
                            change_selected_entity = true;
                        }
                        if(is_selected_keyframe_delete && is_inside_border) 
                        {
                            context.is_stop = true;
                            selected_bone_list.push_back(std::make_pair(bone.second.get(), key));
                        } 
                        
                        else 
                        {
                            if(is_inside_border) 
                            {
                                inside_count++;
                                change_selected_entity = true;
                            }
                            if(is_hovered && selected_frame_count == 0 &&!is_inside_border &&ImGui::IsItemClicked(ImGuiMouseButton_Right)) 
                            {
                                change_selected_entity = true;
                                ImGui::OpenPopup("keyframe_popup");
                            }
                            if(change_selected_entity) 
                            {
                                auto selected_entity = root_entity_->find(bone.second->get_name());
                                if (selected_entity)
                                {
                                    entity_context.is_changed_selected_entity = true;
                                    entity_context.selected_id = selected_entity->get_id();
                                }
                                context.is_stop = true;
                                context.current_frame = ukey;
                                context.is_current_frame_changed = true;
                                if(!is_inside_border) {
                                    ImGui::ItemSelect(name);
                                }
                            }
                        }
                    }        
                    selected_frame_count = inside_count;
                    
                    for(auto &bone : selected_bone_list) 
                    {
                        bone.first->sub_keyframe(bone.second, true);
                    }
                    ImGui::EndNeoTimeLine();
                }
            }
            ImGui::EndNeoGroup();
        }

        draw_keyframe_popup(ui_context);
    }

    void TimelineLayer::draw_keyframe_popup(UiContext &ui_context) 
    {
        if(ImGui::BeginPopup("keyframe_popup")) {
            ImGui::Text("current frame: %d", ui_context.timeline.current_frame); 
            if (ImGui::Button("Delete")&&!ui_context.timeline.is_current_frame_changed) 
            {
                LOG("clicked button");              
                ui_context.timeline.is_delete_current_frame = true;
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
    }
    
    bool TimelineLayer::draw_drag_popup()
    {
        bool is_delete = false;
        if(ImGui::BeginPopup("drag_popup")) {
            ImGui::Text("Selected Keyframe"); 
            if (ImGui::Button("Delete")) 
            {
                is_delete = true;
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
        return is_delete;
    }

}
