#pragma once

#include <functional>
#include <NativeLib/Allocators.h>

namespace nl
{
    namespace raii::methods
    {
        template <typename T>
        static void Release(T obj) { obj->Release(); }

        static void Free(void* obj) { nl::memory::Free(obj); }
    }

    template <typename TObject>
    class Scoped
    {
    public:
        using Callback = std::function<void(TObject)>;

        inline Scoped() :
            m_obj(nullptr),
            m_func(nullptr)
        {
        }

        inline Scoped(TObject obj, Callback func) :
            m_obj(obj),
            m_func(func)
        {
        }

        inline Scoped(Scoped&& other) :
            m_obj(other.m_obj),
            m_func(other.m_func)
        {
            other.m_obj = nullptr;
        }

        inline Scoped(const Scoped&) = delete;

        inline ~Scoped()
        {
            Release();
        }

        inline Scoped& operator =(Scoped&& other)
        {
            m_obj = other.m_obj;
            m_func = other.m_func;
            other.m_obj = nullptr;
            return *this;
        }

        inline Scoped& operator =(const Scoped&) = delete;

        inline TObject& get() { return m_obj; }
        inline const TObject& get() const { return m_obj; }

        inline operator TObject&() { return m_obj; }
        inline operator const TObject&() const { return m_obj; }

        inline TObject& operator ->() { return m_obj; }
        inline const TObject& operator ->() const { return m_obj; }

        inline void Release()
        {
            if (m_obj)
            {
                m_func(m_obj);
                m_obj = nullptr;
            }
        }

        inline void Unhook()
        {
            m_obj = nullptr;
        }

        inline TObject Swap(TObject newValue)
        {
            TObject temp = std::move(m_obj);
            m_obj = newValue;
            return temp;
        }

    private:
        TObject m_obj;
        Callback m_func;
    };

    template <typename TObject>
    class Shared
    {
    public:
        using Callback = std::function<void(TObject)>;

    private:
        struct SharedObject
        {
            TObject Object;
            Callback Function;
            ULONG References;
        };

    public:
        inline Shared() :
            m_shared(nullptr)
        {
        }

        inline Shared(TObject obj, Callback func) :
            m_shared(nullptr)
        {
            m_shared = nl::memory::AllocateThrow(sizeof(SharedObject));
            ZeroMemory(m_shared, sizeof(SharedObject));

            new(&m_shared->Object) TObject(obj);
            new(&m_shared->Function) Callback(func);
            m_shared->References = 1;
        }

        inline Shared(Shared&& other) :
            m_shared(other.m_shared)
        {
            other.m_shared = nullptr;
        }

        inline Shared(const Shared&) = delete;

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

        inline Shared& operator =(const Shared&) = delete;

        inline TObject& get() { return m_shared->Object; }
        inline const TObject& get() const { return m_shared->Object; }

        inline operator TObject&() { return m_shared->Object; }
        inline operator const TObject&() const { return m_shared->Object; }

        inline TObject& operator ->() { return m_shared->Object; }
        inline const TObject& operator ->() const { return m_shared->Object; }

        inline void Release()
        {
            if (m_shared &&
                InterlockedDecrement(&m_shared->References) == 0)
            {
                if (m_shared->Object)
                    m_shared->Function(m_shared->Object);

                m_shared->Function.~Callback();
                nl::memory::Free(m_shared);
                m_shared = nullptr;
            }
        }

        inline void Unhook()
        {
            if (m_shared)
                m_shared->Object = nullptr;
        }

        inline TObject Swap(TObject newValue)
        {
            if (!m_shared)
                throw InvalidOperationException(L"Cannot swap an empty shared object.");

            TObject temp = std::move(m_shared->Object);
            m_shared->Object = newValue;
            return temp;
        }

    private:
        SharedObject* m_shared;
    };

    template <typename T>
    inline Scoped<T> MakeScoped(T value, std::function<void(T)> callback)
    {
        return Scoped<T>(value, callback);
    }

    template <typename T>
    inline Scoped<T> MakeScopedRelease(T value)
    {
        return Scoped<T>(value, &raii::methods::Release<T>);
    }

    template <typename T>
    inline Scoped<T> MakeScopedFree(T value)
    {
        return Scoped<T>(value, &raii::methods::Free);
    }

    template <typename T>
    inline Scoped<T*> MakeScopedDestroy(T* value)
    {
        return MakeScoped<T*>(value, [](auto obj) { nl::memory::Destroy<T>(obj); });
    }

    template <typename T, typename... Args>
    inline Scoped<T*> ConstructScoped(Args&&... args)
    {
        return MakeScopedDestroy<T>(nl::memory::Construct<T>(std::forward<Args>(args)...));
    }

    template <typename T, typename... Args>
    inline Scoped<T*> ConstructScopedThrow(Args&&... args)
    {
        return MakeScopedDestroy<T>(nl::memory::ConstructThrow<T>(std::forward<Args>(args)...));
    }
}

#include "RAII.inl"