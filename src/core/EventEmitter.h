#pragma once
#include <functional>
#include <vector>
#include <algorithm>
#include <utility>

template<typename... Args>
class EventEmitter {
public:
    using Action = std::function<void(Args...)>;
    using ListenerID = std::size_t;

    ListenerID Subscribe(Action action) {
        ListenerID id = ++m_nextId;
        m_listeners.push_back({id, std::move(action), true});
        return id;
    }

    void Unsubscribe(ListenerID id) {
        for (auto& listener : m_listeners) {
            if (listener.id == id) {
                listener.isActive = false;
                m_pendingRemoval = true;
                break;
            }
        }
        
        if (!m_isInvoking && m_pendingRemoval) {
            Cleanup();
        }
    }

    void Invoke(Args... args) {
        m_isInvoking = true;
        for (const auto& listener : m_listeners) {
            if (listener.isActive) {
                listener.action(std::forward<Args>(args)...);
            }
        }
        m_isInvoking = false;

        if (m_pendingRemoval) {
            Cleanup();
        }
    }

    void Clear() {
        m_listeners.clear();
        m_pendingRemoval = false;
    }

private:
    struct Listener {
        ListenerID id;
        Action action;
        bool isActive;
    };

    std::vector<Listener> m_listeners;
    ListenerID m_nextId = 0;
    bool m_isInvoking = false;
    bool m_pendingRemoval = false;

    void Cleanup() {
        m_listeners.erase(
            std::remove_if(m_listeners.begin(), m_listeners.end(),
                [](const Listener& l) { return !l.isActive; }),
            m_listeners.end()
        );
        m_pendingRemoval = false;
    }
};