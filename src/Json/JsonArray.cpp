/*
 * JSON Library by Nicco © 2019
 */

#include "StdAfx.h"

#include <NativeLib/Json.h>

#include "JsonInline.inl"

#include <NativeLib/Allocators.h>

 //DefinePool(JsonArray, 8);

namespace nl
{
    JsonArray::~JsonArray()
    {
    }

    bool JsonArray::Read(const nl::String& json, size_t& i, nl::Vector<nl::String>& parse_errors)
    {
        if (json[i] != '[')
        {
            parse_errors.Add(nl::String::Format("Json at offset {} is not an array", i));
            return false;
        }
        
        ++i;
        while (i < json.GetLength())
        {
            Json_SkipWhitespace(json, i);
            if (i >= json.GetLength())
            {
                parse_errors.Add(nl::String::Format("EOF at {}", i));
                return false;
            }

            if (json[i] == ']')
            {
                ++i;
                return true;
            }
            else if (json[i] == ',')
            {
                ++i;
                continue;
            }

            auto value = Json_ReadValue(json, i, parse_errors);
            if (!value)
                return false;

            m_items.Add(value);
        }

        parse_errors.Add(nl::String::Format("EOF at {}", i));
        return false;
    }

    void JsonArray::AddNull()
    {
        AddBase(ConstructSharedThrow<JsonNull>());
    }

    void JsonArray::AddObject(Shared<JsonBase> obj)
    {
        AddBase(obj);
    }

    Shared<JsonObject> JsonArray::AddObject()
    {
        auto obj = ConstructSharedThrow<JsonObject>();
        AddBase(obj);
        return obj;
    }

    Shared<JsonArray> JsonArray::AddArray()
    {
        auto obj = ConstructSharedThrow<JsonArray>();
        AddBase(obj);
        return obj;
    }

    Shared<JsonBoolean> JsonArray::AddBoolean(bool value)
    {
        auto obj = ConstructSharedThrow<JsonBoolean>(value);
        AddBase(obj);
        return obj;
    }

    Shared<JsonString> JsonArray::AddString(const char* value)
    {
        auto obj = ConstructSharedThrow<JsonString>(value);
        AddBase(obj);
        return obj;
    }

    Shared<JsonNumber> JsonArray::AddNumber(int64_t value)
    {
        auto obj = ConstructSharedThrow<JsonNumber>(value);
        AddBase(obj);
        return obj;
    }

    Shared<JsonNumber> JsonArray::AddNumber(double value)
    {
        auto obj = ConstructSharedThrow<JsonNumber>(value);
        AddBase(obj);
        return obj;
    }
}