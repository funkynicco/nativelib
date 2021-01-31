#include "StdAfx.h"

#include <NativeLib/SystemLayer/SystemLayer.h>

#pragma comment(lib, "DbgHelp.lib")

//#error Rewrite this project to be a Unit Test instead / how to generate output? console write?

void AssertHandler(const nl::assert::Assert& assert)
{
    auto message = nl::String::Format("Expression: {}\nFunction: {}\nFile: {}:{}", assert.Expression, assert.Function, assert.Filename, assert.Line);
    OutputDebugStringA(message);
    OutputDebugStringA("\n");
    MessageBoxA(nullptr, message, "Assert failed", MB_OK | MB_ICONERROR);
}



void* MyAllocHeap(size_t size)
{
    void* ptr = malloc(size + sizeof(size_t));
    *(size_t*)ptr = size;

    ptr = (char*)ptr + sizeof(size_t);

    //////////////////////////////////////////////////////////////////////////
    char buf[1024];
    sprintf_s(buf, "MyAllocHeap(%I64u) ==> %p\n", size, ptr);
    OutputDebugStringA(buf);
    //////////////////////////////////////////////////////////////////////////

    return ptr;
}

void MyFreeHeap(void* ptr)
{
    ptr = (char*)ptr - sizeof(size_t);
    auto size = *(size_t*)ptr;

    //////////////////////////////////////////////////////////////////////////
    char buf[1024];
    sprintf_s(buf, "MyFreeHeap ==> %p (%I64u bytes)\n", (char*)ptr + sizeof(size_t), size);
    OutputDebugStringA(buf);
    //////////////////////////////////////////////////////////////////////////

    memset(ptr, 0xcd, size);
    free(ptr);
}

bool SetupNativeLibSystemLayer()
{
    nl::systemlayer::SystemLayerFunctions functions = {};
    if (!nl::systemlayer::GetDefaultSystemLayerFunctions(&functions))
        return false;
    
    functions.AssertHandler = AssertHandler;
    functions.AllocateHeapMemory = MyAllocHeap;
    functions.FreeHeapMemory = MyFreeHeap;
    
    nl::systemlayer::SetSystemLayerFunctions(&functions);
    return true;
}

void Test()
    {
    auto ccg = nl::memory::Memory::Allocate(1435);
    auto a = nl::memory::Memory::Allocate(1435);

    {
        auto x = nl::memory::Memory(std::move(ccg));
    }

    auto b = nl::memory::Memory(std::move(a));
    auto c = nl::memory::Memory(std::move(b));
    auto d = nl::memory::Memory(std::move(c));
    auto e = nl::memory::Memory(std::move(d));
    auto f = nl::memory::Memory(std::move(e));
    auto g = nl::memory::Memory(std::move(f));
    auto h = nl::memory::Memory(std::move(g));
    auto i = nl::memory::Memory(std::move(h));
    auto j = nl::memory::Memory(std::move(i));
    auto k = nl::memory::Memory(std::move(j));
    auto l = nl::memory::Memory(std::move(k));
    auto m = nl::memory::Memory(std::move(l));
}

int main(int, char**)
    {
    if (!SetupNativeLibSystemLayer())
    {
        std::cout << "Setup NativeLib system layer failed!" << std::endl;
        return 1;
    }

    for (int i = 0; i < 1; i++)
    {
        Test();
    }

    return 0;
}