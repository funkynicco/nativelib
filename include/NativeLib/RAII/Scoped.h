#pragma once

#include <NativeLib/RAII/Methods.h>

#include <functional>

namespace nl
{
    template <typename TObject>
    class Scoped
    {
    public:
        using Callback = std::function<void(TObject*)>;

        inline Scoped() :
            m_obj(nullptr),
            m_func(nullptr)
        {
        }

        inline Scoped(TObject* obj, Callback func) :
            m_obj(obj),
            m_func(func)
        {
        }

        inline Scoped(Scoped&& other) noexcept
        {
            m_obj = other.m_obj;
            m_func = other.m_func;
            other.m_obj = nullptr;
        }

        inline Scoped(const Scoped&) = delete;

        inline ~Scoped()
        {
            Release();
        }

        inline Scoped& operator =(Scoped&& other) noexcept
        {
            Release();

            m_obj = other.m_obj;
            m_func = other.m_func;
            other.m_obj = nullptr;
            return *this;
        }

        inline Scoped& operator =(const Scoped&) = delete;

        inline TObject* get() { return m_obj; }
        inline const TObject* get() const { return m_obj; }

        inline operator TObject*() { return m_obj; }
        inline operator const TObject*() const { return m_obj; }

        inline TObject* operator ->() { return m_obj; }
        inline const TObject* operator ->() const { return m_obj; }

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

        inline TObject* Swap(TObject* newValue)
        {
            TObject* temp = m_obj;
            m_obj = newValue;
            return temp;
        }

    private:
        TObject* m_obj;
        Callback m_func;
    };

    template <typename T>
    inline Scoped<T> MakeScoped(T* value, std::function<void(T*)> callback)
    {
        return Scoped<T>(value, callback);
    }

    template <typename T>
    inline Scoped<T> MakeScopedRelease(T* value)
    {
        return Scoped<T>(value, &raii::methods::Release<T>);
    }

    template <typename T>
    inline Scoped<T> MakeScopedFree(T* value)
    {
        return Scoped<T>(value, &raii::methods::Free);
    }

    template <typename T>
    inline Scoped<T> MakeScopedDestroy(T* value)
    {
        return MakeScoped<T>(value, [](auto obj) { nl::memory::Destroy<T>(obj); });
    }

    template <typename T, typename... Args>
    inline Scoped<T> ConstructScoped(Args&&... args)
    {
        return MakeScopedDestroy<T>(nl::memory::Construct<T>(std::forward<Args>(args)...));
    }

    template <typename T, typename... Args>
    inline Scoped<T> ConstructScopedThrow(Args&&... args)
    {
        return MakeScopedDestroy<T>(nl::memory::ConstructThrow<T>(std::forward<Args>(args)...));
    }
}