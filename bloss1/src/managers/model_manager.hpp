#pragma once

/**
 * @brief Creates, loads and deletes models.
 */

#include "renderer/model.hpp"

namespace bls
{
    class ModelManager
    {
        public:
            void load(const str& name, std::shared_ptr<Model> model);
            std::shared_ptr<Model> get_model(const str& name);
            bool exists(const str& name);

            static ModelManager& get();

        private:
            ModelManager() { }
            ~ModelManager() { }

            std::map<str, std::shared_ptr<Model>> models;
    };
};
