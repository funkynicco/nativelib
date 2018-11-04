#pragma once

#include <NativeLib/RAII/Methods.h>

#include <functional>

namespace nl
{
    namespace shared_internals
    {
        template <typename TObject>
        using Callback = std::function<void(TObject*)>;

        template <typename TObject>
        struct SharedObject
        {
            TObject* Object;
            Callback<TObject> Function;
            ULONG References;

            inline SharedObject(TObject* obj, Callback<TObject> function)
            {
                Object = obj;
                Function = function;
                References = 1;
            }
        };
    }

    template <typename TObject>
    class Shared
    {
    public:
        using CallbackType = nl::shared_internals::Callback<TObject>;
        using SharedObjectType = nl::shared_internals::SharedObject<TObject>;

        inline Shared() :
            m_shared(nullptr)
        {
        }

        inline Shared(TObject* obj, CallbackType func) :
            m_shared(nullptr)
        {
            m_shared = reinterpret_cast<SharedObjectType*>(nl::memory::AllocateThrow(sizeof(SharedObjectType)));
            ZeroMemory(m_shared, sizeof(SharedObjectType));

            new(m_shared) SharedObjectType(obj, func);
        }

        inline Shared(Shared&& other) :
            m_shared(other.m_shared)
        {
            other.m_shared = nullptr;
        }

        inline Shared(const Shared& other)
        {
            InterlockedIncrement(&other.m_shared->References);
            m_shared = other.m_shared;
        }

        inline ~Shared()
        {
            Release();
        }

        inline Shared& operator =(Shared&& other)
        {
            m_shared = other.m_shared;
            other.m_shared = nullptr;
            return *this;
        }

        inline Shared& operator =(const Shared& other)
        {
            InterlockedIncrement(&other.m_shared->References);
            Release();
            m_shared = other.m_shared;
            return *this;
        }

        inline TObject* get() { return m_shared->Object; }
        inline const TObject* get() const { return m_shared->Object; }

        inline operator TObject*() { return m_shared->Object; }
        inline operator const TObject*() const { return m_shared->Object; }

        inline TObject* operator ->() { return m_shared->Object; }
        inline const TObject* operator ->() const { return m_shared->Object; }

        inline void Release()
        {
            if (m_shared &&
                InterlockedDecrement(&m_shared->References) == 0)
            {
                if (m_shared->Object)
                    m_shared->Function(m_shared->Object);

                m_shared->~SharedObject();
                nl::memory::Free(m_shared);
            }

            m_shared = nullptr;
        }

    private:
        SharedObjectType* m_shared;
    };

    template <typename T>
    inline Shared<T> MakeShared(T* value, std::function<void(T*)> callback)
    {
        return Shared<T>(value, callback);
    }

    template <typename T>
    inline Shared<T> MakeSharedRelease(T* value)
    {
        return Shared<T>(value, &raii::methods::Release<T>);
    }

    template <typename T>
    inline Shared<T> MakeSharedFree(T* value)
    {
        return Shared<T>(value, &raii::methods::Free);
    }

    template <typename T>
    inline Shared<T> MakeSharedDestroy(T* value)
    {
        return MakeShared<T>(value, [](auto obj) { nl::memory::Destroy<T>(obj); });
    }

    template <typename T, typename... Args>
    inline Shared<T> ConstructShared(Args&&... args)
    {
        return MakeSharedDestroy<T>(nl::memory::Construct<T>(std::forward<Args>(args)...));
    }

    template <typename T, typename... Args>
    inline Shared<T> ConstructSharedThrow(Args&&... args)
    {
        return MakeSharedDestroy<T>(nl::memory::ConstructThrow<T>(std::forward<Args>(args)...));
    }
}