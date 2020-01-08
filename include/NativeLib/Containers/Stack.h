#pragma once

#include <NativeLib/Containers/Vector.h>

namespace nl
{
    template <typename T, size_t InitialSize>
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
        Vector<T, InitialSize> m_stack;
    };

    template <typename T, size_t InitialSize>
    class Stack : public BaseStack<T, InitialSize>
    {
    public:
        size_t GetCount() const
        {
            return _GetCount();
        }

        void Clear()
        {
            _Clear();
        }

        T GetTop()
        {
            return _GetTop();
        }

        const T GetTop() const
        {
            return _GetTop();
        }

        T Pop()
        {
            return _Pop();
        }

        bool TryPop(T* value)
        {
            return _TryPop(value);
        }

        void Push(const T& value)
        {
            _Push(value);
        }

        void Push(T&& value)
        {
            _Push(std::move(value));
        }
    };
}