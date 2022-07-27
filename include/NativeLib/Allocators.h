#pragma once

#include <NativeLib/Definitions.h>
#include <NativeLib/Exceptions.h>

#include <type_traits>
#include <stdint.h>
#include <utility>

namespace nl
{
    namespace memory
    {
        class Memory
        {
        private:
            struct MemoryDataInfo
            {
                int32_t References;
                size_t Size;
            };

            static MemoryDataInfo s_zeroSizeData;
            static void* s_zeroSizeDataPointer;

        public:
            ~Memory();

            Memory(Memory&& memory) noexcept;
            Memory& operator =(Memory&& memory) noexcept;

            Memory(const Memory& memory) = delete;
            Memory& operator =(const Memory& memory) = delete;

            static Memory Allocate(size_t size);

            operator bool() const { return m_lp != nullptr; }
            operator void* () { return m_lp; }
            operator const void* () const { return m_lp; }

            size_t GetSize() const;

            void* Get() { return m_lp; }
            const void* Get() const { return m_lp; }

            template <typename T>
            T* Get() { return static_cast<T*>(m_lp); }

            template <typename T>
            const T* Get() const { return static_cast<T*>(m_lp); }

        private:
            void* m_lp;

            Memory(void* lp) noexcept;
        };

        template <typename T>
        class GenericMemory
        {
        public:
            GenericMemory(Memory&& mem) noexcept :
                m_memory(std::move(mem))
            {
            }

            GenericMemory(GenericMemory<T>&& mem) noexcept :
                m_memory(std::move(mem.m_memory))
            {
            }

            GenericMemory& operator =(GenericMemory&& other) noexcept
            {
                m_memory = std::move(other.m_memory);
                return *this;
            }

            // unsupported constructors & assignments
            GenericMemory(const GenericMemory&) = delete;
            GenericMemory& operator =(const GenericMemory&) = delete;

            T* Get() { return m_memory.Get<T>(); }
            const T* Get() const { return m_memory.Get<T>(); }

            T* operator ->() { return m_memory.Get<T>(); }
            const T* operator ->() const { return m_memory.Get<T>(); }

            operator T* () { return m_memory.Get<T>(); }
            operator const T* () const { return m_memory.Get<T>(); }

            size_t GetSize() const { return m_memory.GetSize(); }

            static GenericMemory<T> Allocate(size_t bytesSize)
            {
                return GenericMemory<T>(std::move(Memory::Allocate(bytesSize)));
            }

        private:
            Memory m_memory;
        };

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