/*
 * JSON Library by Nicco © 2019
 */

#include "StdAfx.h"

#include <NativeLib/Json.h>

#include "JsonInline.inl"

//DefinePool(JsonObject, 16);

namespace nl
{
    JsonObject::~JsonObject()
    {
        for (auto it : m_members)
        {
            delete it.second;
        }
    }

    bool JsonObject::Read(const std::string& json, size_t& i, std::vector<std::string>& parse_errors)
    {
        char error_str[256];

        if (json[i] != '{')
        {
            sprintf_s(error_str, __FUNCTION__ " - Json at offset %lu is not an array", (unsigned int)i);
            parse_errors.push_back(error_str);
            return false;
        }

        ++i;
        while (i < json.length())
        {
            Json_SkipWhitespace(json, i);
            if (i >= json.length())
            {
                sprintf_s(error_str, __FUNCTION__ " - EOF at %lu", (unsigned int)i);
                parse_errors.push_back(error_str);
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

            std::string name;
            if (!Json_ReadString(name, json, i, parse_errors))
                return false;

            Json_SkipWhitespace(json, i);
            if (i >= json.length())
            {
                sprintf_s(error_str, __FUNCTION__ " - EOF at %lu", (unsigned int)i);
                parse_errors.push_back(error_str);
                return false;
            }

            if (json[i++] != ':')
            {
                sprintf_s(error_str, __FUNCTION__ " - EOF at %lu", (unsigned int)i);
                parse_errors.push_back(error_str);
                return false;
            }

            Json_SkipWhitespace(json, i);
            if (i >= json.length())
            {
                sprintf_s(error_str, __FUNCTION__ " - EOF at %lu", (unsigned int)i);
                parse_errors.push_back(error_str);
                return false;
            }

            auto value = Json_ReadValue(json, i, parse_errors);
            if (!value)
                return false;

            m_members.insert(std::pair<std::string, JsonBase*>(name, value));
        }

        sprintf_s(error_str, __FUNCTION__ " - EOF at %lu", (unsigned int)i);
        parse_errors.push_back(error_str);
        return false;
    }

    void JsonObject::SetNull(const char* pszName)
    {
        SetBase(pszName, new JsonNull);
    }

    void JsonObject::SetObject(const char* pszName, JsonBase* obj)
    {
        SetBase(pszName, obj);
    }

    JsonObject* JsonObject::SetObject(const char* pszName)
    {
        auto obj = new JsonObject;
        SetBase(pszName, obj);
        return obj;
    }

    JsonArray* JsonObject::SetArray(const char* pszName)
    {
        auto obj = new JsonArray;
        SetBase(pszName, obj);
        return obj;
    }

    JsonBoolean* JsonObject::SetBoolean(const char* pszName, bool value)
    {
        auto obj = new JsonBoolean(value);
        SetBase(pszName, obj);
        return obj;
    }

    JsonString* JsonObject::SetString(const char* pszName, const char* value)
    {
        auto obj = new JsonString(value);
        SetBase(pszName, obj);
        return obj;
    }

    JsonNumber* JsonObject::SetNumber(const char* pszName, long long value)
    {
        auto obj = new JsonNumber(value);
        SetBase(pszName, obj);
        return obj;
    }

    JsonNumber* JsonObject::SetNumber(const char* pszName, double value)
    {
        auto obj = new JsonNumber(value);
        SetBase(pszName, obj);
        return obj;
    }
}