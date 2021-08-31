//
// Created by marco.silipo on 31.08.2021.
//

#ifndef GAME_PROJ_EVENT_SOURCE_HPP
#define GAME_PROJ_EVENT_SOURCE_HPP

#include <vector>
#include <shared_mutex>
#include <functional>

namespace vengine::utils
{
    struct [[maybe_unused]] event_args { };
    struct [[maybe_unused]] cancelable_event_args : public event_args {
        bool cancel = false;
    };
    template <class TSource, typename TArg = event_args>
    class event_source
    {
    private:
        struct storage
        {
            std::function<void(TSource&, TArg&)> func;
            size_t id;
            storage(size_t id, std::function<void(TSource&, TArg&)> func) : id(id), func(func) {}
        };
        size_t m_event_id_top;
        std::vector<storage> m_events;
        std::shared_mutex m_mutex;
        friend TSource;

        [[maybe_unused]] void clear()
        {
            std::unique_lock lock(m_mutex);
            m_event_id_top = 0;
            m_events.clear();
        }

        /**
         * Raises this event_source and immediately cancels execution once any
         * of the subscribers set cancel = true.
         *
         * Subscribers are called in the same order they have subscribed.
         *
         * @param source The source of the event.
         * @param arg The event args passed into the event.
         * @returns True if any subscriber cancelled the event. False if no cancellation occurred.
         */
        [[maybe_unused]] bool raise_cancelable(TSource& source, TArg& arg)
        {
            std::shared_lock lock(m_mutex);
            for (auto& it : m_events)
            {
                it.func(source, arg);
                if (arg.cancel)
                {
                    return true;
                }
            }
            return false;
        }

        /**
         * Raises this event_source.
         *
         * Subscribers are called in the same order they have subscribed.
         *
         * @param source The source of the event.
         * @param arg The event args passed into the event.
         */
        [[maybe_unused]] void raise(TSource& source, TArg& arg)
        {
            std::shared_lock lock(m_mutex);
            for (auto& it : m_events)
            {
                it.func(source, arg);
            }
        }
        /**
         * Raises this event_source.
         *
         * Subscribers are called in the same order they have subscribed.
         *
         * @param source The source of the event.
         * @param arg The event args passed into the event.
         */
        [[maybe_unused]] void raise(TSource& source, TArg&& arg)
        {
            std::shared_lock lock(m_mutex);
            for (auto& it : m_events)
            {
                it.func(source, arg);
            }
        }
    public:
        event_source() : m_event_id_top(0), m_events(), m_mutex() {}
        event_source(const event_source&) = delete;

        [[maybe_unused]] size_t subscribe(std::function<void(TSource&, TArg&)> func)
        {
            std::unique_lock lock(m_mutex);
            m_events.push_back({ ++m_event_id_top, func });
            return m_event_id_top;
        }

        [[maybe_unused]] void unsubscribe(size_t event_id)
        {
            std::unique_lock lock(m_mutex);
            auto it = std::find_if(m_events.begin(), m_events.end(), [event_id](const storage& storage) -> bool { return storage.id == event_id; });
            if (m_events.end() != it)
            {
                m_events.erase(it);
            }
        }



    };
}

#endif //GAME_PROJ_EVENT_SOURCE_HPP
