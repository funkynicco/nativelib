#include "StdAfx.h"

#include <NativeLib/Threading/ReadWriteLock.h>
#include <NativeLib/Allocators.h>

//!ALLOW_INCLUDE "Windows.h"

#ifdef NL_PLATFORM_WINDOWS
#include <Windows.h>
#endif

#ifdef NL_PLATFORM_LINUX
#include <shared_mutex>
#endif

namespace nl::threading
{
    ReadWriteLock::ReadWriteLock()
    {
#ifdef NL_PLATFORM_WINDOWS
        m_ptr = SRWLOCK_INIT;
#else
        m_ptr = nl::memory::ConstructThrow<std::shared_mutex>();
#endif
    }

    ReadWriteLock::~ReadWriteLock()
    {
#ifndef NL_PLATFORM_WINDOWS
        nl::memory::Destroy(reinterpret_cast<std::shared_mutex*>(m_ptr));
#endif
    }

    void ReadWriteLock::AcquireShared()
    {
#ifdef NL_PLATFORM_WINDOWS
        AcquireSRWLockShared((PSRWLOCK)&m_ptr);
#else
        reinterpret_cast<std::shared_mutex*>(m_ptr)->lock_shared();
#endif
    }

    void ReadWriteLock::ReleaseShared()
    {
#ifdef NL_PLATFORM_WINDOWS
        ReleaseSRWLockShared((PSRWLOCK)&m_ptr);
#else
        reinterpret_cast<std::shared_mutex*>(m_ptr)->unlock_shared();
#endif
    }

    void ReadWriteLock::AcquireExclusive()
    {
#ifdef NL_PLATFORM_WINDOWS
        AcquireSRWLockExclusive((PSRWLOCK)&m_ptr);
#else
        reinterpret_cast<std::shared_mutex*>(m_ptr)->lock();
#endif
    }

    void ReadWriteLock::ReleaseExclusive()
    {
#ifdef NL_PLATFORM_WINDOWS
        ReleaseSRWLockExclusive((PSRWLOCK)&m_ptr);
#else
        reinterpret_cast<std::shared_mutex*>(m_ptr)->unlock();
#endif
    }
}