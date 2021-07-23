#pragma once

#include <NativeLib/RAII/Methods.h>
#include <NativeLib/Assert.h>
#include <NativeLib/Threading/Interlocked.h>

#include <functional>

namespace nl
{
    namespace shared_internals
    {
        template <typename TObject>
        using Callback = void(*)(TObject*);

        template <typename TObject>
        struct SharedObject
        {
            TObject* Object;
            Callback<TObject> Function;
            int32_t References;

            inline SharedObject(TObject* obj, Callback<TObject> function)
            {
                Object = obj;
                Function = function;
                References = 1;
            }
        };

#ifdef NL_ARCHITECTURE_X64
        constexpr size_t ArchitectureAlignment = 16;
#else
        constexpr size_t ArchitectureAlignment = 8;
#endif

        constexpr size_t GetAlignedSize(size_t target, size_t start = ArchitectureAlignment)
        {
            return start < target ? GetAlignedSize(target, start + ArchitectureAlignment) : start;
        }
    }

    template <typename TObject>
    class Shared
    {
    public:
        using CallbackType = nl::shared_internals::Callback<TObject>;
        using SharedObjectType = nl::shared_internals::SharedObject<TObject>;

        Shared() :
            m_shared(nullptr),
            m_externalShared(false)
        {
        }

        Shared(nullptr_t) :
            m_shared(nullptr),
            m_externalShared(false)
        {
        }

        Shared(SharedObjectType* shared, bool externalShared = true) :
            m_shared(shared),
            m_externalShared(externalShared)
        {
        }

        Shared(TObject* obj, CallbackType func) :
            m_shared(nullptr),
            m_externalShared(false)
        {
            m_shared = reinterpret_cast<SharedObjectType*>(nl::memory::AllocateThrow(sizeof(SharedObjectType)));
            ZeroMemory(m_shared, sizeof(SharedObjectType));

            new(m_shared) SharedObjectType(obj, func);
        }

        Shared(Shared&& other) :
            m_shared(other.m_shared),
            m_externalShared(other.m_externalShared)
        {
            other.m_shared = nullptr;
        }

        Shared(const Shared& other)
        {
            if (other.m_shared)
            {
            nl::threading::Interlocked::Increment(&other.m_shared->References);
            }

            m_shared = other.m_shared;
            m_externalShared = other.m_externalShared;
        }

        template <
            typename TOther,
            std::enable_if_t<std::is_base_of_v<TObject, TOther>, int>* = nullptr>
        Shared(Shared<TOther>&& other) :
            m_shared((SharedObjectType*)other.m_shared),
            m_externalShared(other.m_externalShared)
        {
            other.m_shared = nullptr;
        }

        template <
            typename TOther,
            std::enable_if_t<std::is_base_of_v<TObject, TOther>, int>* = nullptr>
        Shared(const Shared<TOther>& other)
        {
            if (other.m_shared)
            {
            nl::threading::Interlocked::Increment(&other.m_shared->References);
            }

            m_shared = (SharedObjectType*)other.m_shared;
            m_externalShared = other.m_externalShared;
        }

        ~Shared()
        {
            Release();
        }

        Shared& operator =(Shared&& other)
        {
            m_shared = other.m_shared;
            other.m_shared = nullptr;
            m_externalShared = other.m_externalShared;
            return *this;
        }

        Shared& operator =(const Shared& other)
        {
            if (other.m_shared)
            {
            nl::threading::Interlocked::Increment(&other.m_shared->References);
            }

            Release();
            m_shared = (SharedObjectType*)other.m_shared;
            m_externalShared = other.m_externalShared;
            return *this;
        }

        template <
            typename TOther,
            std::enable_if_t<std::is_base_of_v<TObject, TOther>, int>* = nullptr>
        Shared& operator =(Shared<TOther>&& other)
        {
            m_shared = (SharedObjectType*)other.m_shared;
            other.m_shared = nullptr;
            m_externalShared = other.m_externalShared;
            return *this;
        }

        template <
            typename TOther,
            std::enable_if_t<std::is_base_of_v<TObject, TOther>, int>* = nullptr>
        Shared& operator =(const Shared<TOther>& other)
        {
            if (other.m_shared)
            {
            nl::threading::Interlocked::Increment(&other.m_shared->References);
            }

            Release();
            m_shared = (SharedObjectType*)other.m_shared;
            m_externalShared = other.m_externalShared;
            return *this;
        }

        bool has_value() const
        {
            return m_shared != nullptr;
        }

        TObject* get()
        {
            nl_assert_if_debug(m_shared != nullptr);
            return m_shared->Object;
        }

        const TObject* get() const
        {
            nl_assert_if_debug(m_shared != nullptr);
            return m_shared->Object;
        }

        operator TObject* ()
        {
            nl_assert_if_debug(m_shared != nullptr);
            return m_shared->Object;
        }

        operator const TObject* () const
        {
            nl_assert_if_debug(m_shared != nullptr);
            return m_shared->Object;
        }

        TObject* operator ->()
        {
            nl_assert_if_debug(m_shared != nullptr);
            return m_shared->Object;
        }

        const TObject* operator ->() const
        {
            nl_assert_if_debug(m_shared != nullptr);
            return m_shared->Object;
        }

        void Release()
        {
            if (m_shared &&
                nl::threading::Interlocked::Decrement(&m_shared->References) == 0)
            {
                if (m_shared->Object)
                    m_shared->Function(m_shared->Object);

                if (!m_externalShared)
                {
                    m_shared->~SharedObject();
                    nl::memory::Free(m_shared);
                }
            }

            m_shared = nullptr;
        }

        template <
            typename TBase,
            std::enable_if_t<std::is_base_of_v<TBase, TObject>, int> * = nullptr>
            static Shared<TObject> Cast(Shared<TBase>&& base)
        {
            auto shared = base.m_shared;
            base.m_shared = nullptr;
            return Shared<TObject>((SharedObjectType*)shared, base.m_externalShared);
        }

        template <
            typename TBase,
            std::enable_if_t<std::is_base_of_v<TBase, TObject>, int> * = nullptr>
            static Shared<TObject> Cast(const Shared<TBase>& base)
        {
            if (base.m_shared)
            {
            nl::threading::Interlocked::Increment(&base.m_shared->References);
            }

            return Shared<TObject>((SharedObjectType*)base.m_shared, base.m_externalShared);
        }

        SharedObjectType* m_shared;
        bool m_externalShared;
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
        using SharedObject = nl::shared_internals::SharedObject<T>;
        using Callback = nl::shared_internals::Callback<T>;

        auto callback = Callback([](auto obj)
        {
            constexpr size_t sizeOfSharedObject = nl::shared_internals::GetAlignedSize(sizeof(SharedObject));

            auto sharedObject = reinterpret_cast<SharedObject*>(
                reinterpret_cast<unsigned char*>(obj) - sizeOfSharedObject);

            obj->~T();
            sharedObject->~SharedObject();
            nl::memory::Free(sharedObject);
        });

        constexpr size_t sizeOfSharedObject = nl::shared_internals::GetAlignedSize(sizeof(SharedObject));
        unsigned char* memory = reinterpret_cast<unsigned char*>(nl::memory::Allocate(sizeOfSharedObject + sizeof(T)));

        auto obj = new(memory + sizeOfSharedObject) T(std::forward<Args>(args)...);
        return Shared<T>(new(memory) SharedObject(obj, callback));
    }

    template <typename T, typename... Args>
    inline Shared<T> ConstructSharedThrow(Args&&... args)
    {
        using SharedObject = nl::shared_internals::SharedObject<T>;
        using Callback = nl::shared_internals::Callback<T>;

        auto callback = Callback([](auto obj)
        {
            constexpr size_t sizeOfSharedObject = nl::shared_internals::GetAlignedSize(sizeof(SharedObject));

            auto sharedObject = reinterpret_cast<SharedObject*>(
                reinterpret_cast<unsigned char*>(obj) - sizeOfSharedObject);

            obj->~T();
            sharedObject->~SharedObject();
            nl::memory::Free(sharedObject);
        });

        constexpr size_t sizeOfSharedObject = nl::shared_internals::GetAlignedSize(sizeof(SharedObject));
        unsigned char* memory = reinterpret_cast<unsigned char*>(nl::memory::AllocateThrow(sizeOfSharedObject + sizeof(T)));

        T* obj;
        try
        {
            obj = new(memory + sizeOfSharedObject) T(std::forward<Args>(args)...);
        }
        catch (...)
        {
            nl::memory::Free(memory);
            throw;
        }

        return Shared<T>(new(memory) SharedObject(obj, callback));
    }
}