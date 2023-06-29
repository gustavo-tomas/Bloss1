#include "managers/model_manager.hpp"

namespace bls
{
    void ModelManager::load(const str& name, std::shared_ptr<Model> model)
    {
        models[name] = model;
    }

    std::shared_ptr<Model> ModelManager::get_model(const str& name)
    {
        if (exists(name))
            return models[name];

        else
        {
            std::cerr << "model '" << name << "' doesn't exist\n";
            exit(1);
        }
    }

    bool ModelManager::exists(const str& name)
    {
        return models.count(name) > 0;
    }

    ModelManager& ModelManager::get()
    {
        static ModelManager instance;
        return instance;
    }
};
