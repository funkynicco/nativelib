#pragma once

#include <NativeLib/Containers/Vector.h>

namespace nl
{
    template <typename T>
    class BaseStack
    {
    protected:
        BaseStack()
        {
        }

        size_t _GetCount() const
        {
            return m_stack.GetCount();
        }

        void _Clear()
        {
            m_stack.Clear();
        }

        T _GetTop()
        {
            return m_stack[m_stack.GetCount() - 1];
        }

        T _Pop()
        {
            return m_stack.PopLast();
        }

        bool _TryPop(T* value)
        {
            nl_assert_if_debug(value != nullptr);
            if (m_stack.GetCount() == 0)
                return false;

            *value = m_stack.PopLast();
            return true;
        }

        void _Push(const T& value)
        {
            m_stack.Add(value);
        }

        void _Push(T&& value)
        {
            m_stack.Add(std::move(value));
        }

    private:
        Vector<T> m_stack;
    };

    template <typename T>
    class Stack : public BaseStack<T>
    {
        typedef BaseStack<T> super;

    public:
        size_t GetCount() const
        {
            return super::_GetCount();
        }

        void Clear()
        {
            super::_Clear();
        }

        T GetTop()
        {
            return super::_GetTop();
        }

        const T GetTop() const
        {
            return super::_GetTop();
        }

        T Pop()
        {
            return super::_Pop();
        }

        bool TryPop(T* value)
        {
            return super::_TryPop(value);
        }

        void Push(const T& value)
        {
            super::_Push(value);
        }

        void Push(T&& value)
        {
            super::_Push(std::move(value));
        }
    };
}