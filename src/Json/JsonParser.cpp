/*
 * JSON Library by Nicco © 2019
 */

#include "StdAfx.h"

#include <NativeLib/Json.h>

#include "JsonInline.inl"

#include <NativeLib/Allocators.h>

namespace nl
{
    Shared<JsonBase> ParseJson(const char* pszJson, nl::Vector<nl::String>& parse_errors)
    {
        nl::String json = pszJson;

        size_t i = 0;
        Json_SkipWhitespace(json, i);

        if (i >= json.GetLength())
        {
            parse_errors.Add(nl::String::Format("EOF at {}", i));
            return nullptr;
        }

        Shared<JsonBase> obj;

        if (json[i] == '{')
        {
            obj = ConstructSharedThrow<JsonObject>();
            if (!static_cast<JsonObject*>(obj.get())->Read(json, i, parse_errors))
                return nullptr;
        }
        else if (json[i] == '[')
        {
            obj = ConstructSharedThrow<JsonArray>();
            if (!static_cast<JsonArray*>(obj.get())->Read(json, i, parse_errors))
                return nullptr;
        }

        return obj;
    }

    inline bool JsonGenerateProcessBase(nl::String& output, Shared<const JsonBase> pJson)
    {
        if (pJson->GetType() == JsonType::Boolean)
        {
            output.Append(Shared<const JsonBoolean>::Cast(pJson)->GetValue() ? "true" : "false");
        }
        else if (pJson->GetType() == JsonType::String)
        {
            const nl::String& s = Shared<const JsonString>::Cast(pJson)->GetValue();
            output.EnsureCapacity(output.GetCapacity() + s.GetLength() + 2);
            output.Append('"');
            for (size_t i = 0; i < s.GetLength(); ++i)
            {
                char ch = s[i];
                switch (ch)
                {
                case '\\':
                    output.Append('\\');
                    output.Append('\\');
                    break;
                case '"':
                    output.Append('\\');
                    output.Append('"');
                    break;
                case '\t':
                    output.Append('\\');
                    output.Append('t');
                    break;
                case '\r':
                    output.Append('\\');
                    output.Append('r');
                    break;
                case '\n':
                    output.Append('\\');
                    output.Append('n');
                    break;
                default:
                    output.Append(ch);
                    break;
                }
            }

            output.Append('"');
        }
        else if (pJson->GetType() == JsonType::Number)
        {
            char buffer[1024];

            auto num = Shared<const JsonNumber>::Cast(pJson);
            if (num->IsDouble())
                sprintf_s(buffer, "%lf", num->GetDouble());
            else
                sprintf_s(buffer, "%lld", num->GetValue());

            output.Append(buffer);
        }
        else if (pJson->GetType() == JsonType::Array)
        {
            output.Append('[');

            auto pArray = Shared<const JsonArray>::Cast(pJson);
            for (size_t i = 0; i < pArray->GetCount(); ++i)
            {
                if (i > 0)
                    output.Append(',');

                if (!JsonGenerateProcessBase(output, pArray->GetItem(i)))
                    return false;
            }

            output.Append(']');
        }
        else if (pJson->GetType() == JsonType::Object)
        {
            output.Append('{');

            int32_t n = 0;
            for (auto it : Shared<const JsonObject>::Cast(pJson)->GetMembers())
            {
                if (n++ > 0)
                    output.Append(',');

                output.Append('"');
                output.Append(it.first.c_str());
                output.Append('"');
                output.Append(':');

                if (!JsonGenerateProcessBase(output, it.second))
                    return false;
            }

            output.Append('}');
        }
        else if (pJson->GetType() == JsonType::Null)
        {
            output.Append("null");
        }
        else
            return false;

        return true;
    }

    bool GenerateJsonString(nl::String& output, Shared<const JsonBase> pJson)
    {
        output.Clear();

        return JsonGenerateProcessBase(output, pJson); // recursive
    }

    Shared<JsonBase> CreateJsonObject(JsonType type)
    {
        switch (type)
        {
        case JsonType::Null:
            return ConstructSharedThrow<JsonNull>();
        case JsonType::Object:
            return ConstructSharedThrow<JsonObject>();
        case JsonType::Array:
            return ConstructSharedThrow<JsonArray>();
        case JsonType::String:
            return ConstructSharedThrow<JsonString>();
        case JsonType::Number:
            return ConstructSharedThrow<JsonNumber>();
        case JsonType::Boolean:
            return ConstructSharedThrow<JsonBoolean>();
        }

        throw UnsupportedJsonTypeException();
    }
}