#pragma once

#include <NativeLib/RAII/Methods.h>
#include <NativeLib/Assert.h>

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

#ifdef _WIN64
        constexpr size_t ArchitectureAlignment = 16;
#else
        constexpr size_t ArchitectureAlignment = 8;
#endif

        inline constexpr size_t GetAlignedSize(size_t target, size_t start = ArchitectureAlignment)
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

        inline Shared() :
            m_shared(nullptr),
            m_externalShared(false)
        {
        }

        inline Shared(SharedObjectType* shared) :
            m_shared(shared),
            m_externalShared(true)
        {
        }

        inline Shared(TObject* obj, CallbackType func) :
            m_shared(nullptr),
            m_externalShared(false)
        {
            m_shared = reinterpret_cast<SharedObjectType*>(nl::memory::AllocateThrow(sizeof(SharedObjectType)));
            ZeroMemory(m_shared, sizeof(SharedObjectType));

            new(m_shared) SharedObjectType(obj, func);
        }

        inline Shared(Shared&& other) :
            m_shared(other.m_shared),
            m_externalShared(other.m_externalShared)
        {
            other.m_shared = nullptr;
        }

        inline Shared(const Shared& other)
        {
            InterlockedIncrement(&other.m_shared->References);
            m_shared = other.m_shared;
            m_externalShared = other.m_externalShared;
        }

        inline ~Shared()
        {
            Release();
        }

        inline Shared& operator =(Shared&& other)
        {
            m_shared = other.m_shared;
            other.m_shared = nullptr;
            m_externalShared = other.m_externalShared;
            return *this;
        }

        inline Shared& operator =(const Shared& other)
        {
            InterlockedIncrement(&other.m_shared->References);
            Release();
            m_shared = other.m_shared;
            m_externalShared = other.m_externalShared;
            return *this;
        }

        inline TObject* get()
        {
            nl_assert_if_debug(m_shared != nullptr);
            return m_shared->Object;
        }

        inline const TObject* get() const
        {
            nl_assert_if_debug(m_shared != nullptr);
            return m_shared->Object;
        }

        inline operator TObject* ()
        {
            nl_assert_if_debug(m_shared != nullptr);
            return m_shared->Object;
        }

        inline operator const TObject* () const
        {
            nl_assert_if_debug(m_shared != nullptr);
            return m_shared->Object;
        }

        inline TObject* operator ->()
        {
            nl_assert_if_debug(m_shared != nullptr);
            return m_shared->Object;
        }

        inline const TObject* operator ->() const
        {
            nl_assert_if_debug(m_shared != nullptr);
            return m_shared->Object;
        }

        inline void Release()
        {
            if (m_shared &&
                InterlockedDecrement(&m_shared->References) == 0)
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

    private:
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