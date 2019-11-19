/*
 * JSON Library by Nicco © 2019
 */

#include "StdAfx.h"

#include <NativeLib/Json.h>

#include "JsonInline.inl"

namespace nl
{
    std::unique_ptr<JsonBase> ParseJson(const char* pszJson, std::vector<std::string>& parse_errors)
    {
        std::string json = pszJson;

        size_t i = 0;
        Json_SkipWhitespace(json, i);

        if (i >= json.length())
        {
            char error_str[256];
            sprintf_s(error_str, __FUNCTION__ " - EOF at %ld", i);
            parse_errors.push_back(error_str);
            return NULL;
        }

        std::unique_ptr<JsonBase> obj;

        if (json[i] == '{')
        {
            obj = std::unique_ptr<JsonBase>(new JsonObject);
            if (!static_cast<JsonObject*>(obj.get())->Read(json, i, parse_errors))
                return nullptr;
        }
        else if (json[i] == '[')
        {
            obj = std::unique_ptr<JsonBase>(new JsonArray);
            if (!static_cast<JsonArray*>(obj.get())->Read(json, i, parse_errors))
                return nullptr;
        }

        return obj;
    }

    inline bool JsonGenerateProcessBase(std::string& output, const JsonBase* pJson)
    {
        if (pJson->GetType() == JsonType::Boolean)
        {
            output.append(static_cast<const JsonBoolean*>(pJson)->GetValue() ? "true" : "false");
        }
        else if (pJson->GetType() == JsonType::String)
        {
            const std::string& s = static_cast<const JsonString*>(pJson)->GetValue();
            output.reserve(output.capacity() + s.length() + 2);
            output.push_back('"');
            for (size_t i = 0; i < s.length(); ++i)
            {
                char ch = s[i];
                switch (ch)
                {
                case '\\':
                    output.push_back('\\');
                    output.push_back('\\');
                    break;
                case '"':
                    output.push_back('\\');
                    output.push_back('"');
                    break;
                case '\t':
                    output.push_back('\\');
                    output.push_back('t');
                    break;
                case '\r':
                    output.push_back('\\');
                    output.push_back('r');
                    break;
                case '\n':
                    output.push_back('\\');
                    output.push_back('n');
                    break;
                default:
                    output.push_back(ch);
                    break;
                }
            }

            output.push_back('"');
        }
        else if (pJson->GetType() == JsonType::Number)
        {
            char buffer[1024];

            auto num = static_cast<const JsonNumber*>(pJson);
            if (num->IsDouble())
                sprintf_s(buffer, "%lf", num->GetDouble());
            else
                sprintf_s(buffer, "%I64d", num->GetValue());

            output.append(buffer);
        }
        else if (pJson->GetType() == JsonType::Array)
        {
            output.push_back('[');

            auto pArray = static_cast<const JsonArray*>(pJson);
            for (size_t i = 0; i < pArray->GetCount(); ++i)
            {
                if (i > 0)
                    output.push_back(',');

                if (!JsonGenerateProcessBase(output, pArray->GetItem(i)))
                    return false;
            }

            output.push_back(']');
        }
        else if (pJson->GetType() == JsonType::Object)
        {
            output.push_back('{');

            int n = 0;
            auto members = static_cast<const JsonObject*>(pJson)->GetMembers();
            for (auto it = members.cbegin(); it != members.cend(); ++it)
            {
                if (n++ > 0)
                    output.push_back(',');

                output.push_back('"');
                output.append(it->first.c_str());
                output.push_back('"');
                output.push_back(':');

                if (!JsonGenerateProcessBase(output, it->second))
                    return false;
            }

            output.push_back('}');
        }
        else if (pJson->GetType() == JsonType::Null)
        {
            output.append("null");
        }
        else
            return false;

        return true;
    }

    bool GenerateJsonString(std::string& output, const JsonBase* pJson)
    {
        output.clear();

        return JsonGenerateProcessBase(output, pJson); // recursive
    }

    std::unique_ptr<JsonBase> CreateJsonObject(JsonType type)
    {
        switch (type)
        {
        case JsonType::Null:
            return std::unique_ptr<JsonBase>(new JsonNull);
        case JsonType::Object:
            return std::unique_ptr<JsonBase>(new JsonObject);
        case JsonType::Array:
            return std::unique_ptr<JsonBase>(new JsonArray);
        case JsonType::String:
            return std::unique_ptr<JsonBase>(new JsonString);
        case JsonType::Number:
            return std::unique_ptr<JsonBase>(new JsonNumber);
        case JsonType::Boolean:
            return std::unique_ptr<JsonBase>(new JsonBoolean);
        }

        throw UnsupportedJsonTypeException();
    }
}