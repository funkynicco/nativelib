/*
 * JSON Library by Nicco © 2019
 */

#pragma once

#include <NativeLib/Util.h>

namespace nl
{
    inline void Json_SkipWhitespace(const nl::String& json, size_t& i)
    {
        while (i < json.GetLength())
        {
            auto skip = false;

            switch (json[i])
            {
            case ' ':
            case '\t':
            case '\r':
            case '\n':
                skip = true;
                break;
            }

            if (!skip)
                break;

            ++i;
        }
    }

    inline bool Json_CharIsDigit(char ch)
    {
        return ch >= '0' && ch <= '9';
    }

    inline bool Json_ReadNumber(int64_t* pNumber, const nl::String& json, size_t& i)
    {
        int64_t value = 0LL;
        auto first = true;
        auto isNegative = false;

        while (i < json.GetLength())
        {
            if (first &&
                json[i] == '-' &&
                !isNegative)
            {
                isNegative = true;
                ++i;
                continue;
            }

            if (!Json_CharIsDigit(json[i]))
            {
                if (json[i] == '.') // this is a float/double value
                    return false;

                if (!first)
                    *pNumber = isNegative ? -value : value;

                return !first;
            }

            if (!first)
                value *= 10;

            value += (uint8_t)json[i++] - (uint8_t)'0';

            first = false;
        }

        if (isNegative)
            *pNumber = -value;

        *pNumber = value;
        return true;
    }

    inline bool Json_ReadDouble(double* pNumber, const nl::String& json, size_t& i)
    {
        double value = 0;
        auto first = true;
        auto isNegative = false;

        bool isFractional = false;
        int32_t fractions = 0;

        while (i < json.GetLength())
        {
            if (first &&
                json[i] == '-' &&
                !isNegative)
            {
                isNegative = true;
                ++i;
                continue;
            }

            if (json[i] == '.' &&
                !isFractional)
            {
                isFractional = true;
                ++i;
                continue;
            }

            if (!Json_CharIsDigit(json[i]))
                break;

            if (!first)
                value *= 10;

            value += (uint8_t)json[i++] - (uint8_t)'0';
            if (isFractional)
                ++fractions;

            first = false;
        }

        if (first)
            return false;

        value = value / nl::util::Pow(10, fractions);

        if (isNegative)
            *pNumber = -value;

        *pNumber = value;
        return true;
    }

    inline bool Json_ReadString(nl::String& sb, const nl::String& json, size_t& i, nl::Vector<nl::String>& parse_errors) // expects "\"...\""
    {
        if (json[i] != '"')
        {
            parse_errors.Add(nl::String::Format("Json at offset {} is not a string", i));
            return false;
        }

        ++i;
        auto start = i;
        while (i < json.GetLength())
        {
            if (json[i] == '\\')
            {
                if (i + 1 >= json.GetLength())
                {
                    parse_errors.Add(nl::String::Format("EOF at {}", i));
                    return false;
                }

                auto ch = json[++i];
                switch (ch)
                {
                case '"':
                case '\\':
                case '/':
                    sb.Append(ch);
                    break;
                case 'b':
                    sb.Append('\b');
                    break;
                case 'f':
                    sb.Append('\f');
                    break;
                case 'n':
                    sb.Append('\n');
                    break;
                case 'r':
                    sb.Append('\r');
                    break;
                case 't':
                    sb.Append('\t');
                    break;
                case 'u':
                {
                    parse_errors.Add(nl::String::Format("\\u0000 type characters are not supported at offset {}", i));
                    return false;
                }
                }

                ++i;
            }
            else if (json[i] == '"')
            {
                ++i;
                return true;
            }

            sb.Append(json[i++]);
        }

        parse_errors.Add(nl::String::Format("EOF at {}", i));
        return false;
    }

    inline Shared<JsonBase> Json_ReadValue(const nl::String& json, size_t& i, nl::Vector<nl::String>& parse_errors)
    {
        char ch = json[i];

        if (ch == '"')
        {
            nl::String sb;
            if (!Json_ReadString(sb, json, i, parse_errors))
                return nullptr;

            return ConstructSharedThrow<JsonString>(sb);
        }

        if (Json_CharIsDigit(ch) ||
            (ch == '-' &&
                i + 1 < json.GetLength() &&
                Json_CharIsDigit(json[i + 1])))
        {
            size_t temp_i = i;
            int64_t value1;
            if (Json_ReadNumber(&value1, json, i))
                return ConstructSharedThrow<JsonNumber>(value1);

            i = temp_i;
            double value2;
            if (Json_ReadDouble(&value2, json, i))
                return ConstructSharedThrow<JsonNumber>(value2);

            parse_errors.Add(nl::String::Format("Value at offset {} is neither number or double.", temp_i));
            return nullptr;
        }

        if (ch == '{')
        {
            auto obj = ConstructSharedThrow<JsonObject>();
            if (!obj->Read(json, i, parse_errors))
                return nullptr;

            return obj;
        }

        if (ch == '[')
        {
            auto ary = ConstructSharedThrow<JsonArray>();
            if (!ary->Read(json, i, parse_errors))
                return nullptr;

            return ary;
        }

        if (i + 4 <= json.GetLength() &&
            memcmp(json.c_str() + i, "true", 4) == 0)
        {
            i += 4;
            return ConstructSharedThrow<JsonBoolean>(true);
        }

        if (i + 5 <= json.GetLength() &&
            memcmp(json.c_str() + i, "false", 5) == 0)
        {
            i += 5;
            return ConstructSharedThrow<JsonBoolean>(false);
        }

        if (i + 4 <= json.GetLength() &&
            memcmp(json.c_str() + i, "null", 4) == 0)
        {
            i += 4;
            return ConstructSharedThrow<JsonNull>();
        }

        parse_errors.Add(nl::String::Format("No suitable json value found at offset {}", i));
        return nullptr;
    }
}