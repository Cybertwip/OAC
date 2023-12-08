#ifndef ANIM_RESOURCES_IMPORTER_H
#define ANIM_RESOURCES_IMPORTER_H

#include <utility>
#include <memory>
#include <string>
#include <vector>

#include "SmallFBX.h"

struct aiScene;

namespace anim
{
    class Entity;
    class Animation;
    class Model;
    class SharedResources;
    class Importer
    {
    public:
        Importer();
        ~Importer() = default;
        std::pair<std::shared_ptr<Model>, std::vector<std::shared_ptr<Animation>>> read_file(const char *path);
        float mScale = 100.0f;

    private:
        std::shared_ptr<Model> import_model(const sfbx::DocumentPtr doc);
        std::vector<std::shared_ptr<Animation>> import_animation(const sfbx::DocumentPtr doc);

    private:
        std::string path_;
    };
}

#endif
