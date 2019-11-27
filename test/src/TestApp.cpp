#include <iostream>
#include <Windows.h>

#include <NativeLib/Json.h>
#include <NativeLib/Trace/AllocationTrace.h>

int main(int, char**)
{
    HANDLE hFile = CreateFile(L"D:\\map.bin", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);

    LARGE_INTEGER li;
    li.QuadPart = 512;
    SetFilePointerEx(hFile, li, nullptr, FILE_BEGIN);
    SetEndOfFile(hFile);

    {
        HANDLE hFileMapping = CreateFileMapping(hFile, nullptr, PAGE_READWRITE, 0, 0, nullptr);
        char* ptr = (char*)MapViewOfFile(hFileMapping, FILE_MAP_ALL_ACCESS, 0, 0, 0);

        *(int*)ptr = 1234;

        UnmapViewOfFile(ptr);
        CloseHandle(hFileMapping);
    }

    li.QuadPart = 4096;
    SetFilePointerEx(hFile, li, nullptr, FILE_BEGIN);
    SetEndOfFile(hFile);

    {
        HANDLE hFileMapping = CreateFileMapping(hFile, nullptr, PAGE_READWRITE, 0, 0, nullptr);
        char* ptr = (char*)MapViewOfFile(hFileMapping, FILE_MAP_ALL_ACCESS, 0, 0, 0);

        *(int*)ptr = 4321;

        UnmapViewOfFile(ptr);
        CloseHandle(hFileMapping);
    }

    CloseHandle(hFile);
    return 0;

    nl::trace::Setup("trace.bin");

    nl::trace::AddAllocation(__FILE__, __LINE__, __FUNCTION__, nullptr);

    auto obj = nl::CreateJsonObject<nl::JsonObject>();
    obj->SetNull("error");

    std::string json;
    nl::GenerateJsonString(json, obj.get());
    
    printf("%s\n", json.c_str());

    std::vector<std::string> parse_errors;
    obj = nl::ParseJson<nl::JsonObject>(json.c_str(), parse_errors);

    return 0;
}