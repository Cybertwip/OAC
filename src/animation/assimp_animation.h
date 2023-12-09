#ifndef ANIM_ANIMATION_ASSIMP_ANIMATION_H
#define ANIM_ANIMATION_ASSIMP_ANIMATION_H

#include "animation.h"
#include <glm/glm.hpp>

#include <functional>
#include <vector>
#include <map>
#include <iostream>
#include <string>
#include <set>

#include "SmallFBX.h"

namespace anim
{
    class FbxAnimation : public Animation
    {
    public:
		FbxAnimation() = delete;

		FbxAnimation(const sfbx::DocumentPtr doc, const std::shared_ptr<sfbx::AnimationLayer> animationLayer, const std::string& path);

        ~FbxAnimation();

    private:
        void init_animation(const sfbx::DocumentPtr doc, const std::shared_ptr<sfbx::AnimationLayer> animationLayer, const std::string& path);
		void process_bones(const sfbx::AnimationLayer *animation, sfbx::Model *node);
		void process_bindpose(sfbx::Model *node);
		
    };

}

#endif
