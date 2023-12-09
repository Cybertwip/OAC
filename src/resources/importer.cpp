#include "importer.h"
#include <filesystem>
#include "../entity/entity.h"
#include "../animation/json_animation.h"
#include "../animation/assimp_animation.h"
#include "model.h"
#include "../util/log.h"

namespace fs = std::filesystem;

namespace anim
{
    Importer::Importer()
    {
    }

    std::pair<std::shared_ptr<Model>, std::vector<std::shared_ptr<Animation>>> Importer::read_file(const char *path)
    {
        path_ = std::string(path);
        std::vector<std::shared_ptr<Animation>> animations;
        std::shared_ptr<Model> model;
        fs::path fs_path = fs::u8path(path_);

        if (fs_path.extension() == ".json")
        {
            std::shared_ptr<JsonAnimation> anim = std::make_shared<JsonAnimation>(path_.c_str());
            if (anim && anim->get_type() == AnimationType::Json)
            {
                animations.emplace_back(anim);
            }
            return std::make_pair(nullptr, animations);
        }
        try
        {
			sfbx::DocumentPtr doc = sfbx::MakeDocument(path_);
			
			if(doc->valid()){
				model = import_model(doc);
				
				animations = import_animation(doc);
				
				
				if (model == nullptr && animations.size() == 0)
				{
					LOG("ERROR::IMPORTER::NULL:  ANIMATIONS");
				}
				
			} else {
				LOG("ERROR::IMPORTER: INVALID FBX");
			}

			model->set_unit_scale(doc->global_settings.unit_scale);

        }
        catch (std::exception &e)
        {
            LOG("ERROR::IMPORTER: " + std::string(e.what()));
        }
        return std::make_pair(model, animations);
    }
    std::shared_ptr<Model> Importer::import_model(const sfbx::DocumentPtr doc)
    {
        return std::make_shared<Model>(path_, doc);
    }
    std::vector<std::shared_ptr<Animation>> Importer::import_animation(const sfbx::DocumentPtr doc)
    {
        std::vector<std::shared_ptr<Animation>> animations;
		
		if(doc && doc->valid()){
			for(auto& stack : doc->getAnimationStacks()){
				for(auto& animationLayer : stack->getAnimationLayers()){
					animations.push_back(std::make_shared<FbxAnimation>(doc, animationLayer, path_));
				}
			}
		}
		
        return animations;
    }
}
