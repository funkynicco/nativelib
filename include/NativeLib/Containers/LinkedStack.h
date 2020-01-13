#pragma once

#include <NativeLib/Threading/ReadWriteLock.h>

namespace nl
{
    template <typename T>
    class BaseLinkedStack
    {
    protected:
        BaseLinkedStack() :
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
    class LinkedStack : public BaseLinkedStack<T>
    {
        typedef BaseLinkedStack<T> super;

    public:
        T* Pop()
        {
            return super::_Pop();
        }

        bool TryPop(T** ptr)
        {
            return super::_TryPop(ptr);
        }

        void Push(T* ptr)
        {
            super::_Push(ptr);
        }
    };

    template <typename T>
    class SafeLinkedStack : public BaseLinkedStack<T>
    {
        typedef BaseLinkedStack<T> super;

    public:
        T* Pop()
        {
            auto lock = nl::threading::ReadWriteLockScope(&m_lock, true);
            T* ptr = super::_Pop();
            return ptr;
        }

        bool TryPop(T** ptr)
        {
            auto lock = nl::threading::ReadWriteLockScope(&m_lock, true);
            bool res = super::_TryPop(ptr);
            return res;
        }

        void Push(T* ptr)
        {
            auto lock = nl::threading::ReadWriteLockScope(&m_lock, true);
            super::_Push(ptr);
        }

    private:
        nl::threading::ReadWriteLock m_lock;
    };
}