#pragma once

#include <NativeLib/Platform/Windows.h>

namespace nl
{
    template <typename T>
    class BaseStack
    {
    protected:
        BaseStack() :
            m_ptr(nullptr)
        {
        }

        T* _Pop()
        {
            T* ptr = m_ptr;

            if (m_ptr)
                m_ptr = m_ptr->next;

            return ptr;
        }

        bool _TryPop(T** ptr)
        {
            if (!m_ptr)
                return false;

            *ptr = _Pop();
            return true;
        }

        void _Push(T* ptr)
        {
            ptr->next = m_ptr;
            m_ptr = ptr;
        }

    private:
        T* m_ptr;
    };

    template <typename T>
    class Stack : public BaseStack<T>
    {
    public:
        T* Pop()
        {
            return _Pop();
        }

        bool TryPop(T** ptr)
        {
            return _TryPop(ptr);
        }

        void Push(T* ptr)
        {
            _Push(ptr);
        }
    };

    template <typename T>
    class SafeStack : public BaseStack<T>
    {
    public:
        T* Pop()
        {
            AcquireSRWLockExclusive(&m_lock);
            T* ptr = _Pop();
            ReleaseSRWLockExclusive(&m_lock);
            return ptr;
        }

        bool TryPop(T** ptr)
        {
            AcquireSRWLockExclusive(&m_lock);
            bool res = _TryPop(ptr);
            ReleaseSRWLockExclusive(&m_lock);
            return res;
        }

        void Push(T* ptr)
        {
            AcquireSRWLockExclusive(&m_lock);
            _Push(ptr);
            ReleaseSRWLockExclusive(&m_lock);
        }

    private:
        SRWLOCK m_lock = SRWLOCK_INIT;
    };
}