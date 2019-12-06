#include <iostream>
#include <Windows.h>

#include <NativeLib/Json.h>
#include <NativeLib/Trace/AllocationTrace.h>

int main(int, char**)
{
    nl::trace::Setup();

    const int PointerCount = 5000;

    for (;;)
    {
        for (int i = 1; i <= PointerCount; ++i)
        {
            nl::trace::AddAllocation(
                __FILE__,
                __LINE__,
                __FUNCTION__,
                reinterpret_cast<void*>((LONG_PTR)i),
                1024);

            Sleep(0);
        }

        for (int i = 1; i <= PointerCount; ++i)
        {
            nl::trace::RemoveAllocation(reinterpret_cast<void*>((LONG_PTR)i));
            Sleep(0);
        }

        Sleep(2000);
    }

    return 0;
}