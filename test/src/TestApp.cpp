#include "StdAfx.h"

void* AllocateCallback(size_t size) { return malloc(size); }
void* ReallocateCallback(void* ptr, size_t size) { return realloc(ptr, size); }
void FreeCallback(void* ptr) { return free(ptr); }

void AssertHandler(const nl::assert::Assert& assert)
{
    auto message = nl::LargeString<>::Format("Expression: {}\nFunction: {}\nFile: {}:{}", assert.Expression, assert.Function, assert.Filename, assert.Line);
    OutputDebugStringA(message);
    OutputDebugStringA("\n");
    MessageBoxA(nullptr, message, "Assert failed", MB_OK | MB_ICONERROR);
}

int main(int, char**)
{
    nl::assert::SetAssertHandler(AssertHandler);
    nl::memory::SetMemoryManagement(AllocateCallback, ReallocateCallback, FreeCallback);

    nl::parsing::Scanner scanner(R"(

        1
        3
        


        8
        lel

    )");

    while (auto token = scanner.Next())
    {
        std::cout << nl::parsing::TokenTypeToString(token) << ": " << token << " (Line: " << token.GetLine() << ")" << std::endl;
    }

    return 0;
}