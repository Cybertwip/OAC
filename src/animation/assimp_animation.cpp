#include "assimp_animation.h"
#include "../util/utility.h"

#include "SmallFBX.h"

namespace fs = std::filesystem;

namespace anim
{
FbxAnimation::FbxAnimation(const aiAnimation *animation, const aiScene *scene, const char *path)
{
	init_animation(animation, scene, path);
}

FbxAnimation::~FbxAnimation()
{
}

void FbxAnimation::init_animation(const aiAnimation *animation, const aiScene *scene, const char *path)
{
	type_ = AnimationType::Assimp;
	
	path_ = std::string(path);
	fs::path anim_path = fs::u8path(path_);
	name_ = anim_path.filename().string();
	duration_ = animation->mDuration;
	fps_ = animation->mTicksPerSecond;
	
	sfbx::DocumentPtr doc = sfbx::MakeDocument(path_);
	
	auto nodes = doc->getAllObjects();

	auto condition = [](const sfbx::ObjectPtr& nodePtr) {
		return std::dynamic_pointer_cast<sfbx::LimbNode>(nodePtr) != nullptr;
	};

	std::vector<sfbx::ObjectPtr> limbs;

	std::copy_if(nodes.begin(), nodes.end(), std::back_inserter(limbs), condition);
	
	
	auto animationLayer = doc->getAnimationStacks()[0]->getAnimationLayers()[0];
	
	process_bones(animationLayer.get(), doc->getRootModel().get());

	process_bindpose(doc->getRootModel().get());
//
//
//	process_bones(animation, scene->mRootNode);
//	process_bindpose(scene->mRootNode);
}

void FbxAnimation::process_bones(const aiAnimation *animation, const aiNode *root_node)
{
	int size = animation->mNumChannels;
	
	// reading channels(bones engaged in an animation and their keyframes)
	for (int i = 0; i < size; i++)
	{
		auto channel = animation->mChannels[i];
		std::string bone_name = channel->mNodeName.C_Str();
		
		const aiNode *node = root_node->FindNode(channel->mNodeName);
		if (node)
		{

			std::cout << "Assimp : " << bone_name << std::endl;
			
			if(bone_name == "Hips"){
//				name_bone_map_[bone_name]->process_position_scale(channel, glm::inverse(AiMatToGlmMat(node->mTransformation)));
			}
		}
	}
}


void FbxAnimation::process_bones(const sfbx::AnimationLayer *animation, sfbx::Model *node)
{
	auto curveNodes = animation->getAnimationCurveNodes();
	
	auto rotationCondition = [&curveNodes, &node](const std::shared_ptr<sfbx::AnimationCurveNode> curveNode) {
		return curveNode->getAnimationKind() == sfbx::AnimationKind::Rotation && curveNode->getAnimationTarget().get() == node;
	};

	auto positionCondition = [&curveNodes, &node](const std::shared_ptr<sfbx::AnimationCurveNode> curveNode) {
		return curveNode->getAnimationKind() == sfbx::AnimationKind::Position && curveNode->getAnimationTarget().get() == node;
	};
	
	auto scaleCondition = [&curveNodes, &node](const std::shared_ptr<sfbx::AnimationCurveNode> curveNode) {
		return curveNode->getAnimationKind() == sfbx::AnimationKind::Scale && curveNode->getAnimationTarget().get() == node;
	};

	auto positionCurveIter = std::find_if(curveNodes.begin(), curveNodes.end(), positionCondition);

	auto rotationCurveIter = std::find_if(curveNodes.begin(), curveNodes.end(), rotationCondition);

	auto scaleCurveIter = std::find_if(curveNodes.begin(), curveNodes.end(), scaleCondition);

	std::shared_ptr<sfbx::AnimationCurveNode> positionCurve = nullptr;
	std::shared_ptr<sfbx::AnimationCurveNode> rotationCurve = nullptr;
	std::shared_ptr<sfbx::AnimationCurveNode> scaleCurve = nullptr;
	
	
	if(positionCurveIter != curveNodes.end()){
		positionCurve = *positionCurveIter;
	}
	
	if(rotationCurveIter != curveNodes.end()){
		rotationCurve = *rotationCurveIter;
	}
	
	if(scaleCurveIter != curveNodes.end()){
		scaleCurve = *scaleCurveIter;
	}
	
	auto bone_name = std::string{node->getName()};
		
	auto poseMatrix = SfbxMatToGlmMat(node->getLocalMatrix());
	
	name_bone_map_[bone_name] = std::make_unique<Bone>(bone_name, positionCurve.get(), rotationCurve.get(), scaleCurve.get(), glm::inverse(poseMatrix));
	
	for(auto& child : node->getChildren()){
		if(auto model = sfbx::as<sfbx::Model>(child); model){
			process_bones(animation, model.get());
		}
	}
}

void FbxAnimation::process_bindpose(const aiNode *node)
{
	if (node)
	{
		std::string name(node->mName.C_Str());
		auto bindpose = AiMatToGlmMat(node->mTransformation);
		auto children_size = node->mNumChildren;
		name_bindpose_map_[name] = bindpose;
		for (unsigned int i = 0; i < children_size; ++i)
		{
			process_bindpose(node->mChildren[i]);
		}
	}
}


void FbxAnimation::process_bindpose(sfbx::Model *node)
{
	if (node)
	{
		std::string name(node->getName());
		auto bindpose = SfbxMatToGlmMat(node->getLocalMatrix());
		auto children_size = node->getChildren().size();
		name_bindpose_map_[name] = bindpose;
		for (unsigned int i = 0; i < children_size; ++i)
		{
			process_bindpose(sfbx::as<sfbx::Model>(node->getChildren()[i]).get());
		}
	}
}
}
