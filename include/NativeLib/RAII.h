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
}

#include "RAII.inl"