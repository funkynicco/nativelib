#include <iostream>
#include <iomanip>
#include <Windows.h>

#include <unordered_map>

#include <NativeLib/Json.h>
#include <NativeLib/Trace/AllocationTrace.h>
#include <NativeLib/Parsing/Scanner.h>
#include <NativeLib/Allocators.h>

void* AllocateCallback(size_t size) { return malloc(size); }
void* ReallocateCallback(void* ptr, size_t size) { return realloc(ptr, size); }
void FreeCallback(void* ptr) { return free(ptr); }

int main(int, char**)
{
    nl::memory::SetMemoryManagement(AllocateCallback, ReallocateCallback, FreeCallback);

    return 0;
}