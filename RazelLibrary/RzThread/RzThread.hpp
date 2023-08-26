#pragma once

#include "Core/Core.hpp"
#include <thread>
#include <stdexcept>

namespace RzLib
{
    class ScopeThread
    {
    public:
        ScopeThread(std::thread&& thread) : m_thread(std::move(thread))
        {
            if (!m_thread.joinable())
                throw std::logic_error("no thread!");
        }
        ~ScopeThread() { m_thread.detach(); }

        ScopeThread(const ScopeThread& rhs) = delete;
        ScopeThread& operator=(const ScopeThread& rhs) = delete;
    private:
        std::thread m_thread;
    };
}
