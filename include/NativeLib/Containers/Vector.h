#pragma once

#include <NativeLib/Allocators.h>
#include <NativeLib/Assert.h>
#include <NativeLib/Exceptions.h>

namespace nl
{
    template <typename T, size_t InitialSize>
    class Vector
    {
    public:
        const size_t npos = (size_t)-1;

    public:
        class const_iterator
        {
        public:
            const_iterator(const Vector& vector, size_t uIndex) :
                m_vector(vector),
                m_uIndex(uIndex)
            {
            }

            inline bool operator !=(const const_iterator& it) const
            {
                return m_uIndex != it.m_uIndex;
            }

            inline const_iterator& operator ++()
            {
                nl_assert_if_debug(m_uIndex < m_vector.m_uCount);
                ++m_uIndex;
                return *this;
            }

            inline const T& operator *() const
            {
                nl_assert_if_debug(m_uIndex < m_vector.m_uCount);
                return m_vector.m_pArray[m_uIndex];
            }

            inline size_t GetIndex() const { return m_uIndex; }

        private:
            const Vector & m_vector;
            size_t m_uIndex;
        };

        class iterator
        {
        public:
            iterator(Vector& vector, size_t uIndex) :
                m_vector(vector),
                m_uIndex(uIndex)
            {
            }

            inline bool operator !=(const iterator& it) const
            {
                return m_uIndex != it.m_uIndex;
            }

            inline bool operator !=(const const_iterator& it) const
            {
                return m_uIndex != it.GetIndex();
            }

            inline iterator& operator ++()
            {
                nl_assert_if_debug(m_uIndex < m_vector.m_uCount);
                ++m_uIndex;
                return *this;
            }

            inline T& operator *()
            {
                nl_assert_if_debug(m_uIndex < m_vector.m_uCount);
                return m_vector.m_pArray[m_uIndex];
            }

            inline const T& operator *() const
            {
                nl_assert_if_debug(m_uIndex < m_vector.m_uCount);
                return m_vector.m_pArray[m_uIndex];
            }

            inline size_t GetIndex() const { return m_uIndex; }

        private:
            Vector & m_vector;
            size_t m_uIndex;
        };

    public:
        inline Vector()
        {
            m_uCount = 0;
            m_uSize = InitialSize;
            m_pArray = reinterpret_cast<T*>(m_stack);
        }

        inline Vector(const Vector& other)
        {
            m_uCount = other.m_uCount;

            if (m_uCount > InitialSize)
            {
                m_uSize = other.m_uSize;
                m_pArray = reinterpret_cast<T*>(nl::memory::Allocate(m_uSize * sizeof(T)));
            }
            else
            {
                m_uSize = InitialSize;
                m_pArray = reinterpret_cast<T*>(m_stack);
            }

            CopyElements<true>(m_pArray, other.m_pArray, m_uCount);
        }

        inline Vector(Vector&& other)
        {
            m_uCount = other.m_uCount;

            if (m_uCount > InitialSize)
            {
                m_uSize = other.m_uSize;
                m_pArray = other.m_pArray;
                other.m_pArray = reinterpret_cast<T*>(other.m_stack);
                other.m_uCount = 0;
            }
            else
            {
                m_uSize = InitialSize;
                m_pArray = reinterpret_cast<T*>(m_stack);
                MoveElements<true>(m_pArray, other.m_pArray, m_uCount);
                other.m_uCount = 0;
            }
        }

        inline ~Vector()
        {
            Clear();

            if (m_pArray != reinterpret_cast<T*>(m_stack))
                nl::memory::Free(m_pArray);
        }

        inline Vector& operator =(const Vector& vector)
        {
            Clear();
            PrepareAdd(vector.m_uCount);

            m_uCount = vector.m_uCount;
            CopyElements<true>(m_pArray, vector.m_pArray, m_uCount);
            return *this;
        }

        inline Vector& operator =(Vector&& vector)
        {
            Clear();

            if (vector.m_pArray != reinterpret_cast<T*>(vector.m_stack))
            {
                if (m_pArray != reinterpret_cast<T*>(m_stack))
                    nl::memory::Free(m_pArray);

                m_pArray = vector.m_pArray;
                m_uSize = vector.m_uSize;
                m_uCount = vector.m_uCount;
                vector.m_pArray = reinterpret_cast<T*>(vector.m_stack);
                vector.m_uCount = 0;
            }
            else
            {
                PrepareAdd(vector.m_uCount);

                m_uCount = vector.m_uCount;
                MoveElements<true>(m_pArray, vector.m_pArray, vector.m_uCount);

                vector.m_uCount = 0;
            }

            return *this;
        }

        inline T& operator[](size_t uIndex)
        {
            nl_assert_if_debug(uIndex < m_uCount);
            return m_pArray[uIndex];
        }

        inline const T& operator[](size_t uIndex) const
        {
            nl_assert_if_debug(uIndex < m_uCount);
            return m_pArray[uIndex];
        }

        inline const T* GetArray() const { return m_pArray; }
        inline size_t GetCount() const { return m_uCount; }
        inline size_t GetSize() const { return m_uSize; }

        inline iterator begin() { return iterator(*this, 0); }
        inline const_iterator begin() const { return const_iterator(*this, 0); }
        inline iterator end() { return iterator(*this, m_uCount); }
        inline const_iterator end() const { return const_iterator(*this, m_uCount); }

        inline void Clear()
        {
            if constexpr (!std::is_trivial_v<T>)
            {
                for (size_t i = 0; i < m_uCount; ++i)
                {
                    m_pArray[i].~T();
                }
            }

            m_uCount = 0;
        }

        inline void Shrink(bool compact)
        {
            if (m_pArray == reinterpret_cast<T*>(m_stack))
                return;

            size_t uNewSize = m_uCount;
            if (!compact)
            {
                uNewSize = 2;
                while (uNewSize < m_uCount)
                    uNewSize <<= 1;
            }

            if (uNewSize >= m_uSize)
                return;

            T* pNewArray = reinterpret_cast<T*>(m_stack);
            if (uNewSize > InitialSize)
            {
                pNewArray = reinterpret_cast<T*>(nl::memory::Allocate(uNewSize * sizeof(T)));
                if (!pNewArray)
                    throw BadAllocationException();
            }

            MoveElements<true>(pNewArray, m_pArray, m_uCount);

            if (m_pArray != reinterpret_cast<T*>(m_stack))
                nl::memory::Free(m_pArray);

            m_pArray = pNewArray;
            m_uSize = uNewSize;
        }

        inline void Reserve(size_t size)
        {
            if (m_uSize > size)
                return;

            Expand(size - m_uSize);
        }

        inline void PrepareAdd(size_t uCount)
        {
            size_t uNewSize = m_uCount + uCount;
            if (uNewSize <= m_uSize)
                return;

            Expand(uNewSize - m_uSize);
        }

        inline void Add(const T& value)
        {
            if (m_uCount == m_uSize)
                Expand(1);

            if constexpr (std::is_trivial_v<T>)
                m_pArray[m_uCount++] = value;
            else
                new(&m_pArray[m_uCount++]) T(value);
        }

        inline void Add(T&& value)
        {
            if (m_uCount == m_uSize)
                Expand(1);

            if constexpr (std::is_trivial_v<T>)
                m_pArray[m_uCount++] = std::move(value);
            else
                new(&m_pArray[m_uCount++]) T(std::move(value));
        }

        inline void Delete(size_t uIndex)
        {
            nl_assert(uIndex < m_uCount);

            --m_uCount;

            if constexpr (!std::is_trivial_v<T>)
            {
                for (size_t i = uIndex; i < m_uCount; ++i)
                {
                    m_pArray[i] = std::move(m_pArray[i + 1]);
                }

                m_pArray[m_uCount].~T();
            }
            else
                memcpy(&m_pArray[uIndex], &m_pArray[uIndex + 1], (m_uCount - uIndex) * sizeof(T));
        }

        inline T Erase(size_t uIndex)
        {
            nl_assert(uIndex < m_uCount);

            --m_uCount;

            T result = std::move(m_pArray[uIndex]);

            if constexpr (!std::is_trivial_v<T>)
            {
                for (size_t i = uIndex; i < m_uCount; ++i)
                {
                    m_pArray[i] = std::move(m_pArray[i + 1]);
                }

                m_pArray[m_uCount].~T();
            }
            else
                memcpy(&m_pArray[uIndex], &m_pArray[uIndex + 1], (m_uCount - uIndex) * sizeof(T));

            return result;
        }

        inline Vector CopyRange(size_t uIndex, size_t uCount) const
        {
            nl_assert(uIndex + uCount <= m_uCount);

            Vector result;

            result.PrepareAdd(uCount);

            result.CopyElements<true>(result.m_pArray, &m_pArray[uIndex], uCount);
            result.m_uCount = uCount;

            return result;
        }

        inline size_t Find(const T& obj)
        {
            for (size_t i = 0; i < m_uCount; ++i)
            {
                if (m_pArray[i] == obj)
                    return i;
            }

            return npos;
        }

        inline T PopFirst()
        {
            nl_assert(m_uCount != 0);

            --m_uCount;
            T obj = std::move(m_pArray[0]);

            Delete(0);

            return obj;
        }

        inline T PopLast()
        {
            nl_assert(m_uCount != 0);

            T obj = std::move(m_pArray[--m_uCount]);

            if constexpr (!std::is_trivial_v<T>)
                m_pArray[m_uCount].~T();

            return obj;
        }

    private:
        template <bool UseNew>
        inline void CopyElements(T* pNew, const T* pOld, size_t uCount)
        {
            if constexpr (!std::is_trivial_v<T>)
            {
                for (size_t i = 0; i < uCount; ++i)
                {
                    if constexpr (UseNew)
                        new (&pNew[i]) T(pOld[i]);
                    else
                        pNew[i] = pOld[i];
                }
            }
            else
                memcpy(pNew, pOld, uCount * sizeof(T));
        }

        template <bool UseNew>
        inline void MoveElements(T* pNew, T* pOld, size_t uCount)
        {
            if constexpr (!std::is_trivial_v<T>)
            {
                for (size_t i = 0; i < uCount; ++i)
                {
                    if constexpr (UseNew)
                        new (&pNew[i]) T(std::move(pOld[i]));
                    else
                        pNew[i] = std::move(pOld[i]);

                    pOld[i].~T();
                }
            }
            else
                memcpy(pNew, pOld, uCount * sizeof(T));
        }

        inline void Expand(size_t uAdd)
        {
            size_t uRequiredSize = m_uSize + uAdd;
            size_t uNewSize = m_uSize;

            if (uNewSize == 0)
                uNewSize = 1;

            while (uNewSize < uRequiredSize)
                uNewSize *= 2;

            T* pNewArray = reinterpret_cast<T*>(nl::memory::Allocate(uNewSize * sizeof(T)));
            if (!pNewArray)
                throw BadAllocationException();

            MoveElements<true>(pNewArray, m_pArray, m_uCount);

            if (m_pArray != reinterpret_cast<T*>(m_stack))
                nl::memory::Free(m_pArray);

            m_pArray = pNewArray;
            m_uSize = uNewSize;
        }

        size_t m_uSize;
        size_t m_uCount;

        T* m_pArray;
        char m_stack[InitialSize * sizeof(T)];
    };
}