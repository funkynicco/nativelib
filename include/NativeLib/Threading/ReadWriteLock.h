#pragma once

namespace nl::threading
{
    class ReadWriteLock
    {
    public:
        ReadWriteLock();
        ~ReadWriteLock();

        void AcquireShared();
        void ReleaseShared();

        void AcquireExclusive();
        void ReleaseExclusive();

    private:
        void* m_ptr;
    };

    class ReadWriteLockScope
    {
    public:
        ReadWriteLockScope(ReadWriteLock* pRWLock, bool exclusive) :
            m_pRWLock(pRWLock),
            m_exclusive(exclusive)
        {
            if (m_exclusive)
                m_pRWLock->AcquireExclusive();
            else
                m_pRWLock->AcquireShared();
        }

        ~ReadWriteLockScope()
        {
            Release();
        }

        void Release()
        {
            if (m_exclusive)
                m_pRWLock->ReleaseExclusive();
            else
                m_pRWLock->ReleaseShared();
        }

    private:
        ReadWriteLock* m_pRWLock;
        bool m_exclusive;
    };
}