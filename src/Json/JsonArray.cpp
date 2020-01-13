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
        for (auto it : m_items)
        {
            nl::memory::Destroy(it);
        }
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
        AddBase(nl::memory::ConstructThrow<JsonNull>());
    }

    void JsonArray::AddObject(JsonBase* obj)
    {
        AddBase(obj);
    }

    JsonObject* JsonArray::AddObject()
    {
        auto obj = nl::memory::ConstructThrow<JsonObject>();
        AddBase(obj);
        return obj;
    }

    JsonArray* JsonArray::AddArray()
    {
        auto obj = nl::memory::ConstructThrow<JsonArray>();
        AddBase(obj);
        return obj;
    }

    JsonBoolean* JsonArray::AddBoolean(bool value)
    {
        auto obj = nl::memory::ConstructThrow<JsonBoolean>(value);
        AddBase(obj);
        return obj;
    }

    JsonString* JsonArray::AddString(const char* value)
    {
        auto obj = nl::memory::ConstructThrow<JsonString>(value);
        AddBase(obj);
        return obj;
    }

    JsonNumber* JsonArray::AddNumber(int64_t value)
    {
        auto obj = nl::memory::ConstructThrow<JsonNumber>(value);
        AddBase(obj);
        return obj;
    }

    JsonNumber* JsonArray::AddNumber(double value)
    {
        auto obj = nl::memory::ConstructThrow<JsonNumber>(value);
        AddBase(obj);
        return obj;
    }
}