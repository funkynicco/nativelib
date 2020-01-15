#pragma once

//#include <type_traits>
#include <utility> // for std pair

#include <NativeLib/SystemLayer/SystemLayer.h>

#include <NativeLib/Containers/Vector.h>

// TODO: write a proper Map, with bucket index that has a linked list of the nodes
// TODO: #error this crashes when begin() + i is called due to comparing to end fails..

namespace nl
{
    template <typename TKey, typename TValue>
    class Map
    {
    public:    
        using map_type = nl::Vector<std::pair<TKey, TValue>>;
        using iterator = typename map_type::iterator;
        using const_iterator = typename map_type::const_iterator;

        bool Add(const TKey& key, const TValue& value);
        bool Contains(const TKey& key) const;
        bool Remove(const TKey& key);
        iterator Remove(iterator it);
        void Clear() { m_map.Clear(); }

        size_t GetCount() const { return m_map.GetCount(); }

        typename map_type::iterator find(const TKey& key);
        typename map_type::const_iterator find(const TKey& key) const;

        typename map_type::iterator begin() { return m_map.begin(); }
        typename map_type::iterator end() { return m_map.end(); }

        typename map_type::const_iterator begin() const { return m_map.begin(); }
        typename map_type::const_iterator end() const { return m_map.end(); }

    private:
        map_type m_map;
    };
}

#define NL_ALLOW_MAP_INL_INCLUDE_CONTEXT
#include "Map.inl"
#undef NL_ALLOW_MAP_INL_INCLUDE_CONTEXT

/*
    // Algorithm for bucket size

    static int PtrMapGetBucketIndex(LPPTR_MAP lpMap, DWORD dwKey)
    {
    #if defined(_WIN64)
        static_assert(sizeof(size_t) == 8, "This code is for 64-bit size_t.");
        const size_t _FNV_offset_basis = 14695981039346656037ULL;
        const size_t _FNV_prime = 1099511628211ULL;
    #else
        static_assert(sizeof(size_t) == 4, "This code is for 32-bit size_t.");
        const size_t _FNV_offset_basis = 2166136261U;
        const size_t _FNV_prime = 16777619U;
    #endif

        const unsigned char* data = (const unsigned char*)&dwKey;

        size_t result = _FNV_offset_basis;
        for (size_t i = 0; i < sizeof(dwKey); ++i)
        {
            result ^= (size_t)data[i];
            result *= _FNV_prime;
        }

        return result & lpMap->dwMask;
    }

    static void* PtrMapFind(LPPTR_MAP lpMap, DWORD dwKey)
    {
        LPPTR_MAP_ENTRY ptr = lpMap->Table[PtrMapGetBucketIndex(lpMap, dwKey)];

        while (ptr)
        {
            if (ptr->dwKey == dwKey)
                return ptr->Ptr;

            ptr = ptr->next;
        }

        return NULL;
    }
*/