/*
 * JSON Library by Nicco © 2019
 */

#include "StdAfx.h"

#include <NativeLib/Json.h>

//!ALLOW_INCLUDE "JsonInline.inl"
#include "JsonInline.inl"

#include <NativeLib/Allocators.h>

//DefinePool(JsonObject, 16);

namespace nl
{
    JsonObject::~JsonObject()
    {
    }

    bool JsonObject::Read(const nl::String& json, size_t& i, nl::Vector<nl::String>& parse_errors)
    {
        if (json[i] != '{')
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

            if (json[i] == '}')
            {
                ++i;
                return true;
            }
            else if (json[i] == ',')
            {
                ++i;
                continue;
            }

            nl::String name;
            if (!Json_ReadString(name, json, i, parse_errors))
                return false;

            Json_SkipWhitespace(json, i);
            if (i >= json.GetLength())
            {
                parse_errors.Add(nl::String::Format("EOF at {}", i));
                return false;
            }

            if (json[i++] != ':')
            {
                parse_errors.Add(nl::String::Format("EOF at {}", i));
                return false;
            }

            Json_SkipWhitespace(json, i);
            if (i >= json.GetLength())
            {
                parse_errors.Add(nl::String::Format("EOF at {}", i));
                return false;
            }

            auto value = Json_ReadValue(json, i, parse_errors);
            if (!value)
                return false;

            m_members.Add(name, value);
        }

        parse_errors.Add(nl::String::Format("EOF at {}", i));
        return false;
    }

    void JsonObject::SetNull(const char* pszName)
    {
        SetBase(pszName, ConstructSharedThrow<JsonNull>());
    }

    void JsonObject::SetObject(const char* pszName, Shared<JsonBase> obj)
    {
        SetBase(pszName, obj);
    }

    Shared<JsonObject> JsonObject::SetObject(const char* pszName)
    {
        auto obj = ConstructSharedThrow<JsonObject>();
        SetBase(pszName, obj);
        return obj;
    }

    Shared<JsonArray> JsonObject::SetArray(const char* pszName)
    {
        auto obj = ConstructSharedThrow<JsonArray>();
        SetBase(pszName, obj);
        return obj;
    }

    Shared<JsonBoolean> JsonObject::SetBoolean(const char* pszName, bool value)
    {
        auto obj = ConstructSharedThrow<JsonBoolean>(value);
        SetBase(pszName, obj);
        return obj;
    }

    Shared<JsonString> JsonObject::SetString(const char* pszName, const char* value)
    {
        auto obj = ConstructSharedThrow<JsonString>(value);
        SetBase(pszName, obj);
        return obj;
    }

    Shared<JsonNumber> JsonObject::SetNumber(const char* pszName, int64_t value)
    {
        auto obj = ConstructSharedThrow<JsonNumber>(value);
        SetBase(pszName, obj);
        return obj;
    }

    Shared<JsonNumber> JsonObject::SetNumber(const char* pszName, double value)
    {
        auto obj = ConstructSharedThrow<JsonNumber>(value);
        SetBase(pszName, obj);
        return obj;
    }
}