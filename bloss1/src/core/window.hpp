#pragma once

/**
 * @brief Interface for a generic window. The platform must implement the window accordingly.
 */

#include "core/core.hpp"
#include "core/event.hpp"

namespace bls
{
    class Window
    {
        public:
            virtual ~Window() { }

            virtual void update() = 0;
            virtual u32 get_width() const = 0;
            virtual u32 get_height() const = 0;

            virtual void* get_native_window() const = 0;
            virtual f64 get_time() const = 0;

            // Must be implemented by the platform
            static Window* create(const str& title, const u32& width, const u32& height);

            // protected:
            // @TODO: encapsulate this
            template<typename EventType>
            void fire_event(const EventType& event)
            {
                EventSystem::fire_event(event);
            }
    };
};
