#include "pch.hpp"
#include "Core/RzThread.hpp"

namespace RzLib
{
    ScopeThread::ScopeThread(std::thread&& thread)
        : m_thread(std::move(thread))
    {
        if (!m_thread.joinable())
            throw std::logic_error("no thread!");
    }
    ScopeThread::~ScopeThread()
    {
        m_thread.detach();
    }
}
