#ifndef ANIM_ANIMATION_ASSIMP_ANIMATION_H
#define ANIM_ANIMATION_ASSIMP_ANIMATION_H

#include "animation.h"
#include <glm/glm.hpp>
#include <assimp/scene.h>

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

		FbxAnimation(const aiAnimation *animation, const aiScene *scene, const char *path);

        ~FbxAnimation();

    private:
        void init_animation(const aiAnimation *animation, const aiScene *scene, const char *path);
		void process_bones(const sfbx::AnimationLayer *animation, sfbx::Model *node);
        void process_bones(const aiAnimation *animation, const aiNode *root_node);
		void process_bindpose(const aiNode *node);
		void process_bindpose(sfbx::Model *node);
		
    };

}

#endif
