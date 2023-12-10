#ifndef ANIM_ENTITY_ENTITY_H
#define ANIM_ENTITY_ENTITY_H

#include <vector>
#include <memory>
#include <string>

#include "components/component.h"
#include "components/transform_component.h"
#include "../util/log.h"

#include <imgui/ImSequencer.h>
#include <imgui/ImCurveEdit.h>
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

namespace anim
{

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

    class Entity
    {
    public:
        bool is_deactivate_ = false;
        Entity() = default;
        Entity(const std::string &name, int id, Entity *parent = nullptr);
        template <class T>
        T *get_component();
        template <class T>
        T *add_component();
        void update();
        Entity *find(const std::string &name);
        Entity *find(Entity *entity, const std::string &name);

        void set_name(const std::string &name);
        void add_children(std::shared_ptr<Entity> &children);
        void set_sub_child_num(int num);
        const std::string &get_name()
        {
            return name_;
        }
        const glm::mat4 &get_local() const
        {
            return local_;
        }
        void set_local(const glm::mat4 &local)
        {
            local_ = local;
        }
        std::vector<std::shared_ptr<Entity>> &get_mutable_children()
        {
            return children_;
        }
		
		std::vector<std::shared_ptr<Entity>> get_children_recursive() {
			// Clear the result vector before populating it
			std::vector<std::shared_ptr<Entity>> result;
			
			// Start the recursive process
			get_children_recursive_internal(result);
			
			return result;
		}

		void get_children_recursive_internal(std::vector<std::shared_ptr<Entity>>& result) {
			// Add the immediate children of this entity to the result vector
			std::vector<std::shared_ptr<Entity>> immediateChildren = get_mutable_children();
			result.insert(result.end(), immediateChildren.begin(), immediateChildren.end());
			
			// Recursively call the function on each child
			for (auto& child : immediateChildren) {
				child->get_children_recursive_internal(result);
			}
		}
		

        void set_world_transformation(const glm::mat4 &world)
        {
            world_ = world;
        }
        const glm::mat4 &get_world_transformation() const
        {
            return world_;
        }
        void set_parent(Entity *entity)
        {
            parent_ = entity;
        }
        void set_root(Entity *entity)
        {
            root_ = entity;
        }
        Entity *get_mutable_parent()
        {
            return parent_;
        }
        Entity *get_mutable_root()
        {
            return root_;
        }
        int get_id()
        {
            return id_;
        }
        void set_is_selected(bool is_selected)
        {
            is_selected_ = is_selected;
        }
        bool is_selected_{false};
		
		TracksSequencer& get_tracks_sequencer() {
			return sequencer_;
		}

    private:
        std::string name_{};
        Entity *parent_ = nullptr;
        Entity *root_ = nullptr;
        std::vector<std::shared_ptr<Component>> components_{};
        std::vector<std::shared_ptr<Entity>> children_{};
        glm::mat4 world_{1.0f};
        glm::mat4 local_{1.0f};
        int id_{-1};
		
		TracksSequencer sequencer_;
    };

    template <class T>
    T *Entity::get_component()
    {
        for (auto &component : components_)
        {
            if (component->get_type() == T::type)
            {
                return static_cast<T *>(component.get());
            }
        }
        return nullptr;
    }
    template <class T>
    T *Entity::add_component()
    {
        auto cmp = get_component<T>();
        if (cmp)
        {
            return cmp;
        }
        components_.push_back(std::make_shared<T>());
        return static_cast<T *>(components_.back().get());
    }
}

#endif
