#pragma once

#include <NativeLib/Threading/ReadWriteLock.h>

namespace nl
{
    template <typename T>
    class BaseQueue
    {
    protected:
        BaseQueue() :
            m_pHead(nullptr),
            m_pTail(nullptr)
        {
        }

        T* _PopHead()
        {
            T* ptr = m_pHead;
            if (ptr == nullptr)
                return nullptr;

            m_pHead = m_pHead->next;
            if (m_pHead)
                m_pHead->prev = nullptr;
            else
                m_pTail = nullptr;

            return ptr;
        }

        T* _PopTail()
        {
            T* ptr = m_pTail;
            if (ptr == nullptr)
                return nullptr;

            m_pTail = m_pTail->prev;
            if (m_pTail)
                m_pTail->next = nullptr;
            else
                m_pHead = nullptr;

            return ptr;
        }

        bool _TryPopHead(T** ptr)
        {
            if (m_pHead == nullptr)
                return false;

            *ptr = _PopHead();
            return true;
        }

        bool _TryPopTail(T** ptr)
        {
            if (m_pTail == nullptr)
                return false;

            *ptr = _PopTail();
            return true;
        }

        void _AddHead(T* ptr)
        {
            ptr->prev = nullptr;
            ptr->next = m_pHead;

            if (m_pHead)
                m_pHead->prev = ptr;

            m_pHead = ptr;

            if (m_pTail == nullptr)
                m_pTail = ptr;
        }

        void _AddTail(T* ptr)
        {
            ptr->prev = m_pTail;
            ptr->next = nullptr;

            if (m_pTail)
                m_pTail->next = ptr;

            m_pTail = ptr;

            if (m_pHead == nullptr)
                m_pHead = ptr;
        }

    private:
        T* m_pHead;
        T* m_pTail;
    };

    template <typename T>
    class Queue : public BaseQueue<T>
    {
    public:
        T* PopHead()
        {
            return _PopHead();
        }

        T* PopTail()
        {
            return _PopTail();
        }

        bool TryPopHead(T** ptr)
        {
            return _TryPopHead(ptr);
        }

        bool TryPopTail(T** ptr)
        {
            return _TryPopTail(ptr);
        }

        void AddHead(T* ptr)
        {
            _AddHead(ptr);
        }

        void AddTail(T* ptr)
        {
            _AddTail(ptr);
        }
    };

    template <typename T>
    class SafeQueue : public BaseQueue<T>
    {
    public:
        T* PopHead()
        {
            auto lock = nl::threading::ReadWriteLockScope(&m_lock, true);
            T* ptr = _PopHead();
            return ptr;
        }

        T* PopTail()
        {
            auto lock = nl::threading::ReadWriteLockScope(&m_lock, true);
            T* ptr = _PopTail();
            return ptr;
        }

        bool TryPopHead(T** ptr)
        {
            auto lock = nl::threading::ReadWriteLockScope(&m_lock, true);
            bool res = _TryPopHead(ptr);
            return res;
        }

        bool TryPopTail(T** ptr)
        {
            auto lock = nl::threading::ReadWriteLockScope(&m_lock, true);
            bool res = _TryPopTail(ptr);
            return res;
        }

        void AddHead(T* ptr)
        {
            auto lock = nl::threading::ReadWriteLockScope(&m_lock, true);
            _AddHead(ptr);
        }

        void AddTail(T* ptr)
        {
            auto lock = nl::threading::ReadWriteLockScope(&m_lock, true);
            _AddTail(ptr);
        }

    private:
        nl::threading::ReadWriteLock m_lock;
    };
}