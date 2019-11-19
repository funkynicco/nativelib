#include <iostream>
#include <Windows.h>

#include <NativeLib/Json.h>

int main(int, char**)
{
    auto obj = nl::CreateJsonObject<nl::JsonObject>();
    obj->SetNull("error");

    std::string json;
    nl::GenerateJsonString(json, obj.get());
    
    printf("%s\n", json.c_str());

    std::vector<std::string> parse_errors;
    obj = nl::ParseJson<nl::JsonObject>(json.c_str(), parse_errors);

    return 0;
}