#include <iostream>
#include <Windows.h>

#include <NativeLib/Json.h>
#include <NativeLib/Trace/AllocationTrace.h>

int main(int, char**)
{
    nl::trace::Setup();

    const int PointerCount = 5000;

    LONG_PTR nextPtr = 1;

    for (;;)
    {
        for (int i = 1; i <= PointerCount; ++i)
        {
            nl::trace::X_AddAllocation(
                __FILE__,
                __LINE__,
                __FUNCTION__,
                reinterpret_cast<void*>(nextPtr++),
                1024);

            Sleep(0);
        }

        Sleep(2000);

#if 1
        for (LONG_PTR i = 1; i < nextPtr; ++i)
        {
            nl::trace::X_RemoveAllocation(reinterpret_cast<void*>(i));
            Sleep(1);
        }

        nextPtr = 1;
#endif

        Sleep(2000);
    }

    return 0;
}