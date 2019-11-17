/*
 * JSON Library by Nicco © 2019
 */

#include "StdAfx.h"

#include <NativeLib/Json/Json.h>

#include "JsonInline.inl"

 //DefinePool(JsonArray, 8);

namespace nl
{
    JsonArray::~JsonArray()
    {
        for (auto it : m_items)
        {
            delete it;
        }
    }

    bool JsonArray::Read(const std::string& json, size_t& i, std::vector<std::string>& parse_errors)
    {
        char error_str[256];

        if (json[i] != '[')
        {
            sprintf_s(error_str, __FUNCTION__ " - Json at offset %ld is not an array", i);
            parse_errors.push_back(error_str);
            return false;
        }

        ++i;
        while (i < json.length())
        {
            Json_SkipWhitespace(json, i);
            if (i >= json.length())
            {
                sprintf_s(error_str, __FUNCTION__ " - EOF at %ld", i);
                parse_errors.push_back(error_str);
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

            m_items.push_back(value);
        }

        sprintf_s(error_str, __FUNCTION__ " - EOF at %ld", i);
        parse_errors.push_back(error_str);
        return false;
    }

    void JsonArray::AddObject(JsonBase* obj)
    {
        AddBase(obj);
    }

    JsonObject* JsonArray::AddObject()
    {
        auto obj = new JsonObject;
        AddBase(obj);
        return obj;
    }

    JsonArray* JsonArray::AddArray()
    {
        auto obj = new JsonArray;
        AddBase(obj);
        return obj;
    }

    JsonBoolean* JsonArray::AddBoolean(bool value)
    {
        auto obj = new JsonBoolean(value);
        AddBase(obj);
        return obj;
    }

    JsonString* JsonArray::AddString(const char* value)
    {
        auto obj = new JsonString(value);
        AddBase(obj);
        return obj;
    }

    JsonNumber* JsonArray::AddNumber(long long value)
    {
        auto obj = new JsonNumber(value);
        AddBase(obj);
        return obj;
    }

    JsonNumber* JsonArray::AddNumber(double value)
    {
        auto obj = new JsonNumber(value);
        AddBase(obj);
        return obj;
    }
}