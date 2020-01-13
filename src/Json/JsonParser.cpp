/*
 * JSON Library by Nicco © 2019
 */

#include "StdAfx.h"

#include <NativeLib/Json.h>

#include "JsonInline.inl"

#include <NativeLib/Allocators.h>

namespace nl
{
    nl::Scoped<JsonBase> ParseJson(const char* pszJson, nl::Vector<nl::String>& parse_errors)
    {
        nl::String json = pszJson;

        size_t i = 0;
        Json_SkipWhitespace(json, i);

        if (i >= json.GetLength())
        {
            parse_errors.Add(nl::String::Format("EOF at {}", i));
            return nullptr;
        }

        nl::Scoped<JsonBase> obj;

        if (json[i] == '{')
        {
            obj = nl::MakeScopedDestroy<JsonBase>(nl::memory::ConstructThrow<JsonObject>());
            if (!static_cast<JsonObject*>(obj.get())->Read(json, i, parse_errors))
                return nullptr;
        }
        else if (json[i] == '[')
        {
            obj = nl::MakeScopedDestroy<JsonBase>(nl::memory::ConstructThrow<JsonArray>());
            if (!static_cast<JsonArray*>(obj.get())->Read(json, i, parse_errors))
                return nullptr;
        }

        return obj;
    }

    inline bool JsonGenerateProcessBase(nl::String& output, const JsonBase* pJson)
    {
        if (pJson->GetType() == JsonType::Boolean)
        {
            output.Append(static_cast<const JsonBoolean*>(pJson)->GetValue() ? "true" : "false");
        }
        else if (pJson->GetType() == JsonType::String)
        {
            const nl::String& s = static_cast<const JsonString*>(pJson)->GetValue();
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

            auto num = static_cast<const JsonNumber*>(pJson);
            if (num->IsDouble())
                sprintf_s(buffer, "%lf", num->GetDouble());
            else
                sprintf_s(buffer, "%lld", num->GetValue());

            output.Append(buffer);
        }
        else if (pJson->GetType() == JsonType::Array)
        {
            output.Append('[');

            auto pArray = static_cast<const JsonArray*>(pJson);
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
            for (auto it : static_cast<const JsonObject*>(pJson)->GetMembers())
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

    bool GenerateJsonString(nl::String& output, const JsonBase* pJson)
    {
        output.Clear();

        return JsonGenerateProcessBase(output, pJson); // recursive
    }

    nl::Scoped<JsonBase> CreateJsonObject(JsonType type)
    {
        switch (type)
        {
        case JsonType::Null:
            return nl::MakeScopedDestroy<JsonBase>(nl::memory::ConstructThrow<JsonNull>());
        case JsonType::Object:
            return nl::MakeScopedDestroy<JsonBase>(nl::memory::ConstructThrow<JsonObject>());
        case JsonType::Array:
            return nl::MakeScopedDestroy<JsonBase>(nl::memory::ConstructThrow<JsonArray>());
        case JsonType::String:
            return nl::MakeScopedDestroy<JsonBase>(nl::memory::ConstructThrow<JsonString>());
        case JsonType::Number:
            return nl::MakeScopedDestroy<JsonBase>(nl::memory::ConstructThrow<JsonNumber>());
        case JsonType::Boolean:
            return nl::MakeScopedDestroy<JsonBase>(nl::memory::ConstructThrow<JsonBoolean>());
        }

        throw UnsupportedJsonTypeException();
    }
}