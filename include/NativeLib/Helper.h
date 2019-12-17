#pragma once

#include <NativeLib/Assert.h>

namespace nl
{
    namespace linkedlist
    {
        template <typename T>
        void AddHead(T** head, T** tail, T* child)
        {
            nl_assert_if_debug(child->prev == nullptr && child->next == nullptr);

            child->prev = nullptr;
            child->next = *head;
            if (*head)
                (*head)->prev = child;

            *head = child;

            if (*tail == nullptr)
                *tail = child;
        }

        template <typename T>
        void AddTail(T** head, T** tail, T* child)
        {
            nl_assert_if_debug(child->prev == nullptr && child->next == nullptr);

            child->prev = *tail;
            child->next = nullptr;
            if (*tail)
                (*tail)->next = child;
            
            *tail = child;

            if (*head == nullptr)
                *head = child;
        }

        template <typename T>
        void Remove(T** head, T** tail, T* child)
        {
            //nl_assert_if_debug(child->prev != nullptr && child->next != nullptr);

            if (child->prev)
                child->prev->next = child->next;

            if (child->next)
                child->next->prev = child->prev;

            if (*head == child)
                *head = (*head)->next;

            if (*tail == child)
                *tail = (*tail)->prev;

            child->prev = nullptr;
            child->next = nullptr;
        }

        template <typename T>
        T* PopHead(T** head, T** tail)
        {
            if (*head == nullptr)
                return nullptr;

            T* child = *head;
            Remove(head, tail, child);
            return child;
        }

        template <typename T>
        T* PopTail(T** head, T** tail)
        {
            if (*tail == nullptr)
                return nullptr;

            T* child = *tail;
            Remove(head, tail, child);
            return child;
        }
    }
}