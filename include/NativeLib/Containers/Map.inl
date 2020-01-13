#pragma once

#ifndef NL_ALLOW_MAP_INL_INCLUDE_CONTEXT
#error Cannot include Map.inl directly
#endif

namespace nl
{
    template <typename TKey, typename TValue>
    inline bool Map<TKey, TValue>::Add(const TKey& key, const TValue& value)
    {
        if (Contains(key))
            return false;

        m_map.Add(std::pair<TKey, TValue>(key, value));
        return false;
    }

    template <typename TKey, typename TValue>
    inline bool Map<TKey, TValue>::Contains(const TKey& key) const
    {
        for (size_t i = 0; i < m_map.GetCount(); i++)
        {
            if (m_map[i].first == key)
                return true;
        }

        return false;
    }
    
    template <typename TKey, typename TValue>
    inline bool Map<TKey, TValue>::Remove(const TKey& key)
    {
        for (size_t i = 0; i < m_map.GetCount(); i++)
        {
            if (m_map[i].first == key)
            {
                m_map.Delete(i);
                return true;
            }
        }

        return false;
    }

    template <typename TKey, typename TValue>
    inline typename Map<TKey, TValue>::iterator Map<TKey, TValue>::Remove(iterator it)
    {
        auto index = it.GetIndex();
        m_map.Delete(index);
        return m_map.begin() + index;
    }

    template <typename TKey, typename TValue>
    inline typename Map<TKey, TValue>::map_type::iterator Map<TKey, TValue>::find(const TKey& key)
    {
        for (size_t i = 0; i < m_map.GetCount(); i++)
        {
            if (m_map[i].first == key)
                return begin() + i;
        }

        return end();
    }
    
    template <typename TKey, typename TValue>
    inline typename Map<TKey, TValue>::map_type::const_iterator Map<TKey, TValue>::find(const TKey& key) const
    {
        for (size_t i = 0; i < m_map.GetCount(); i++)
        {
            if (m_map[i].first == key)
                return begin() + i;
        }

        return end();
    }
}