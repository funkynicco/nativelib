#include <iostream>
#include <Windows.h>

#include <NativeLib/Json.h>
#include <NativeLib/Trace/AllocationTrace.h>

int main(int, char**)
{
    nl::trace::Setup();

    for (int i = 1; i < 100; ++i)
    {
        nl::trace::AddAllocation(
            __FILE__,
            __LINE__,
            __FUNCTION__,
            main,
            1024);

        Sleep(1000);

        nl::trace::RemoveAllocation(main);

        Sleep(1000);
    }

    return 0;
}