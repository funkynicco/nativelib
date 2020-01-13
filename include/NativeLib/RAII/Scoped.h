#pragma once

#include <NativeLib/RAII/Methods.h>
#include <NativeLib/Assert.h>

#include <functional>

namespace nl
{
    template <typename TObject>
    class Scoped
    {
    public:
        using Callback = std::function<void(TObject*)>;

        Scoped() :
            m_obj(nullptr),
            m_func(nullptr)
        {
        }

        Scoped(nullptr_t) :
            m_obj(nullptr),
            m_func(nullptr)
        {
        }

        Scoped(TObject* obj, Callback func) :
            m_obj(obj),
            m_func(func)
        {
        }

        Scoped(Scoped&& other) noexcept
        {
            m_obj = other.m_obj;
            m_func = other.m_func;
            other.m_obj = nullptr;
        }

        Scoped(const Scoped&) = delete;

        ~Scoped()
        {
            Release();
        }

        Scoped& operator =(Scoped&& other) noexcept
        {
            Release();

            m_obj = other.m_obj;
            m_func = other.m_func;
            other.m_obj = nullptr;
            return *this;
        }

        Scoped& operator =(const Scoped&) = delete;

        TObject* get()
        {
            nl_assert_if_debug(m_obj != nullptr);
            return m_obj;
        }

        const TObject* get() const
        {
            nl_assert_if_debug(m_obj != nullptr);
            return m_obj;
        }

        operator TObject* ()
        {
            nl_assert_if_debug(m_obj != nullptr);
            return m_obj;
        }

        operator const TObject* () const
        {
            nl_assert_if_debug(m_obj != nullptr);
            return m_obj;
        }

        TObject* operator ->()
        {
            nl_assert_if_debug(m_obj != nullptr);
            return m_obj;
        }

        const TObject* operator ->() const
        {
            nl_assert_if_debug(m_obj != nullptr);
            return m_obj;
        }

        void Release()
        {
            if (m_obj)
            {
                m_func(m_obj);
                m_obj = nullptr;
            }
        }

        void Unhook()
        {
            m_obj = nullptr;
        }

        TObject* Swap(TObject* newValue)
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