/*
 * JSON Library by Nicco © 2019
 */

#include "StdAfx.h"

#include <NativeLib/Json.h>

#include "JsonInline.inl"

#include <NativeLib/Allocators.h>

//DefinePool(JsonObject, 16);

namespace nl
{
    JsonObject::~JsonObject()
    {
        for (auto it : m_members)
        {
            nl::memory::Destroy(it.second);
        }
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
        SetBase(pszName, nl::memory::ConstructThrow<JsonNull>());
    }

    void JsonObject::SetObject(const char* pszName, JsonBase* obj)
    {
        SetBase(pszName, obj);
    }

    JsonObject* JsonObject::SetObject(const char* pszName)
    {
        auto obj = nl::memory::ConstructThrow<JsonObject>();
        SetBase(pszName, obj);
        return obj;
    }

    JsonArray* JsonObject::SetArray(const char* pszName)
    {
        auto obj = nl::memory::ConstructThrow<JsonArray>();
        SetBase(pszName, obj);
        return obj;
    }

    JsonBoolean* JsonObject::SetBoolean(const char* pszName, bool value)
    {
        auto obj = nl::memory::ConstructThrow<JsonBoolean>(value);
        SetBase(pszName, obj);
        return obj;
    }

    JsonString* JsonObject::SetString(const char* pszName, const char* value)
    {
        auto obj = nl::memory::ConstructThrow<JsonString>(value);
        SetBase(pszName, obj);
        return obj;
    }

    JsonNumber* JsonObject::SetNumber(const char* pszName, int64_t value)
    {
        auto obj = nl::memory::ConstructThrow<JsonNumber>(value);
        SetBase(pszName, obj);
        return obj;
    }

    JsonNumber* JsonObject::SetNumber(const char* pszName, double value)
    {
        auto obj = nl::memory::ConstructThrow<JsonNumber>(value);
        SetBase(pszName, obj);
        return obj;
    }
}