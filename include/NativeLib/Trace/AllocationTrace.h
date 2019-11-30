#pragma once

namespace nl
{
    namespace trace
    {
        void Setup();
        void Terminate();

        __declspec(noinline) void AddAllocation(const char* filename, int line, const char* function, void* ptr, size_t sizeOfPtrData);
        __declspec(noinline) void RemoveAllocation(void* ptr);
    }
}