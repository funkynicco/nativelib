#include <iostream>
#include <Windows.h>

#include <NativeLib/Json.h>

int main(int, char**)
{
    auto obj = nl::CreateJsonObject<nl::JsonObject>();
    obj->SetString("hello", "world");

    std::string json;
    nl::GenerateJsonString(json, obj);

    printf("%s\n", json.c_str());
    return 0;
}