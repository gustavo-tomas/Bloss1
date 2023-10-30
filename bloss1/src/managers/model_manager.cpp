#include "managers/model_manager.hpp"

namespace bls
{
    void ModelManager::load(const str &name, std::shared_ptr<Model> model)
    {
        models[name] = model;
    }

    std::shared_ptr<Model> ModelManager::get_model(const str &name)
    {
        if (exists(name))
            return models[name];

        else
            throw std::runtime_error("model '" + name + "' doesn't exist");
    }

    bool ModelManager::exists(const str &name)
    {
        return models.count(name) > 0;
    }

    ModelManager &ModelManager::get()
    {
        static ModelManager instance;
        return instance;
    }
};  // namespace bls
