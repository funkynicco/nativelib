#pragma once

namespace nl
{
    namespace trace
    {
        void Setup();
        void Terminate();

        __declspec(noinline) void X_AddAllocation(const char* filename, int line, const char* function, void* ptr, size_t sizeOfPtrData);
        __declspec(noinline) void X_RemoveAllocation(void* ptr);

        // Snaps the bytes to the nearest virtual paged size.
        size_t SnapToVirtualPage(size_t size);
    }
}