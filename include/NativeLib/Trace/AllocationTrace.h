#pragma once

#ifdef NL_PLATFORM_WINDOWS

namespace nl
{
    namespace trace
    {
        void Setup();
        void Terminate();

        NL_NOINLINE void AddAllocation(const char* filename, int32_t line, const char* function, void* ptr, size_t sizeOfPtrData);
        NL_NOINLINE void RemoveAllocation(void* ptr);

        // Snaps the bytes to the nearest virtual paged size.
        size_t SnapToVirtualPage(size_t size);
    }
}

#endif