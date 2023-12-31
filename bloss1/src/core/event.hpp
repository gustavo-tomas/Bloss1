#pragma once

/**
 * @brief The event callback system. The callbacks are registered according to each event type.
 * When an event occurs, the callbacks for that event are called in order of registration.
 */

#include "core/core.hpp"

namespace bls
{
    class EventSystem
    {
        public:
            template <typename EventType>
            static void register_callback(const std::function<void(const EventType &)> &callback)
            {
                get_callbacks<EventType>().push_back(callback);
            }

        private:
            friend class Game;  // Don't judge me

            template <typename EventType>
            static void fire_event(const EventType &event)
            {
                // Run all callbacks for the specified event
                for (const auto &callback : get_callbacks<EventType>()) callback(event);
            }

            template <typename EventType>
            static std::vector<std::function<void(const EventType &)>> &get_callbacks()
            {
                static std::vector<std::function<void(const EventType &)>> callbacks;
                return callbacks;
            }
    };

    struct Event
    {
            virtual ~Event()
            {
            }
    };

    struct WindowCloseEvent : public Event
    {
            // Empty
    };

    struct WindowResizeEvent : public Event
    {
            WindowResizeEvent(u32 width, u32 height) : width(width), height(height)
            {
            }

            u32 width;
            u32 height;
    };

    struct KeyPressEvent : public Event
    {
            KeyPressEvent(u32 key) : key(key)
            {
            }

            u32 key;
    };

    struct MouseMoveEvent : public Event
    {
            MouseMoveEvent(f64 x_position, f64 y_position) : x_position(x_position), y_position(y_position)
            {
            }

            f64 x_position;
            f64 y_position;
    };

    struct MouseScrollEvent : public Event
    {
            MouseScrollEvent(f64 x_offset, f64 y_offset) : x_offset(x_offset), y_offset(y_offset)
            {
            }

            f64 x_offset;
            f64 y_offset;
    };
};  // namespace bls
