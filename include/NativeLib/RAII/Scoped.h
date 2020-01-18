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
        using Callback = void(*)(TObject*);

        Scoped() :
            m_obj(nullptr),
            m_deleter(nullptr)
        {
        }

        Scoped(nullptr_t) :
            m_obj(nullptr),
            m_deleter(nullptr)
        {
        }

        Scoped(TObject* obj, Callback func) :
            m_obj(obj),
            m_deleter(func)
        {
        }

        Scoped(Scoped&& other) noexcept
        {
            m_obj = other.m_obj;
            m_deleter = other.m_deleter;
            other.m_obj = nullptr;
        }

        template <
            typename TOther,
            std::enable_if_t<std::is_base_of_v<TObject, TOther>, int>* = nullptr>
        Scoped(Scoped<TOther>&& other) noexcept
        {
            m_obj = other.m_obj;
            m_deleter = (Callback)other.m_deleter;
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
            m_deleter = other.m_deleter;
            other.m_obj = nullptr;
            return *this;
        }

        template <
            typename TOther,
            std::enable_if_t<std::is_base_of_v<TObject, TOther>, int>* = nullptr>
        Scoped& operator =(Scoped<TOther>&& other) noexcept
        {
            Release();

            m_obj = other.m_obj;
            m_deleter = (Callback)other.m_deleter;
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
                m_deleter(m_obj);
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

        template <
            typename TBase,
            std::enable_if_t<std::is_base_of_v<TBase, TObject>, int>* = nullptr>
        static Scoped<TObject> Cast(Scoped<TBase>&& base)
        {
            auto obj = base.m_obj;
            base.m_obj = nullptr;
            return Scoped<TObject>((TObject*)obj, (Callback)base.m_deleter);
        }

        TObject* m_obj;
        Callback m_deleter;
    };

    template <typename T>
    inline Scoped<T> MakeScoped(T* value, typename Scoped<T>::Callback callback)
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