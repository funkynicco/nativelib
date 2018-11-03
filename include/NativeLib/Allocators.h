#pragma once

#include <NativeLib/Definitions.h>
#include <NativeLib/Exceptions.h>

namespace nl
{
    namespace memory
    {
        typedef void*(*AllocateCallback)(size_t size);
        typedef void*(*ReallocateCallback)(void* ptr, size_t size);
        typedef void(*FreeCallback)(void* ptr);

        void SetMemoryManagement(AllocateCallback allocate, ReallocateCallback reallocate, FreeCallback free);

        void* Allocate(size_t size);
        void* AllocateThrow(size_t size);
        void* Reallocate(void* ptr, size_t size);
        void Free(void* ptr);

        template <typename T, typename... Args>
        inline T* Construct(Args&&... args)
        {
            auto ptr = Allocate(sizeof(T));
            if (!ptr)
                return nullptr;

            try
            {
                return new(ptr) T(std::forward<Args>(args)...);
            }
            catch (...)
            {
                Free(ptr);
                throw;
            }
        }

        template <typename T, typename... Args>
        inline T* ConstructThrow(Args&&... args)
        {
            auto ptr = Allocate(sizeof(T));
            if (!ptr)
                throw BadAllocationException();

            try
            {
                return new(ptr) T(std::forward<Args>(args)...);
            }
            catch (...)
            {
                Free(ptr);
                throw;
            }
        }

        template <typename T>
        inline void Destroy(T* value)
        {
            value->~T();
            Free(value);
        }
    }
}