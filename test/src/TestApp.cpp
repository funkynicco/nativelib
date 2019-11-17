#include <iostream>
#include <Windows.h>

#include <NativeLib/String.h>

int main(int, char**)
{
    nl::String<> test;
    test = "hello world\n";

    printf(test.c_str());

    return 0;
}