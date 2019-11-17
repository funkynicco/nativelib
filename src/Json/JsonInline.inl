/*
 * JSON Library by Nicco © 2019
 */

#pragma once

namespace nl
{
    inline void Json_SkipWhitespace(const std::string& json, size_t& i)
    {
        while (i < json.length())
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

    inline bool Json_ReadNumber(__int64* pNumber, const std::string& json, size_t& i)
    {
        __int64 value = 0LL;
        auto first = true;
        auto isNegative = false;

        while (i < json.length())
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

            value += (unsigned char)json[i++] - (unsigned char)'0';

            first = false;
        }

        if (isNegative)
            *pNumber = -value;

        *pNumber = value;
        return true;
    }

    inline bool Json_ReadDouble(double* pNumber, const std::string& json, size_t& i)
    {
        double value = 0;
        auto first = true;
        auto isNegative = false;

        bool isFractional = false;
        int fractions = 0;

        while (i < json.length())
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

            value += (unsigned char)json[i++] - (unsigned char)'0';
            if (isFractional)
                ++fractions;

            first = false;
        }

        if (first)
            return false;

        value = value / pow(10, fractions);

        if (isNegative)
            *pNumber = -value;

        *pNumber = value;
        return true;
    }

    inline bool Json_ReadString(std::string& sb, const std::string& json, size_t& i, std::vector<std::string>& parse_errors) // expects "\"...\""
    {
        char error_str[256];
        if (json[i] != '"')
        {
            sprintf_s(error_str, __FUNCTION__ " - Json at offset %ld is not a string", i);
            parse_errors.push_back(error_str);
            return false;
        }

        ++i;
        auto start = i;
        while (i < json.length())
        {
            if (json[i] == '\\')
            {
                if (i + 1 >= json.length())
                {
                    sprintf_s(error_str, __FUNCTION__ " - EOF at %ld", i);
                    parse_errors.push_back(error_str);
                    return false;
                }

                auto ch = json[++i];
                switch (ch)
                {
                case '"':
                case '\\':
                case '/':
                    sb.push_back(ch);
                    break;
                case 'b':
                    sb.push_back('\b');
                    break;
                case 'f':
                    sb.push_back('\f');
                    break;
                case 'n':
                    sb.push_back('\n');
                    break;
                case 'r':
                    sb.push_back('\r');
                    break;
                case 't':
                    sb.push_back('\t');
                    break;
                case 'u':
                {
                    sprintf_s(error_str, __FUNCTION__ " - \\u0000 type characters are not supported at offset %ld", i);
                    parse_errors.push_back(error_str);
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

            sb.push_back(json[i++]);
        }

        sprintf_s(error_str, __FUNCTION__ " - EOF at %ld", i);
        parse_errors.push_back(error_str);
        return false;
    }

    inline JsonBase* Json_ReadValue(const std::string& json, size_t& i, std::vector<std::string>& parse_errors)
    {
        char error_str[256];
        char ch = json[i];

        if (ch == '"')
        {
            std::string sb;
            if (!Json_ReadString(sb, json, i, parse_errors))
                return NULL;

            return new JsonString(sb);
        }

        if (Json_CharIsDigit(ch) ||
            (ch == '-' &&
                i + 1 < json.length() &&
                Json_CharIsDigit(json[i + 1])))
        {
            size_t temp_i = i;
            __int64 value1;
            if (Json_ReadNumber(&value1, json, i))
                return new JsonNumber(value1);

            i = temp_i;
            double value2;
            if (Json_ReadDouble(&value2, json, i))
                return new JsonNumber(value2);

            sprintf_s(error_str, __FUNCTION__ " - Value at offset %ld is neither number or double.", temp_i);
            parse_errors.push_back(error_str);
            return NULL;
        }

        if (ch == '{')
        {
            auto obj = new JsonObject();
            if (!obj->Read(json, i, parse_errors))
            {
                delete obj;
                return NULL;
            }

            return obj;
        }

        if (ch == '[')
        {
            auto ary = new JsonArray();
            if (!ary->Read(json, i, parse_errors))
            {
                delete ary;
                return NULL;
            }

            return ary;
        }

        if (i + 4 <= json.length() &&
            _memicmp(json.c_str() + i, "true", 4) == 0)
        {
            i += 4;
            return new JsonBoolean(true);
        }

        if (i + 5 <= json.length() &&
            _memicmp(json.c_str() + i, "false", 5) == 0)
        {
            i += 5;
            return new JsonBoolean(false);
        }

        if (i + 4 <= json.length() &&
            _memicmp(json.c_str() + i, "null", 4) == 0)
        {
            i += 4;
            return NULL;
        }

        sprintf_s(error_str, __FUNCTION__ " - No suitable json value found at offset %ld", i);
        parse_errors.push_back(error_str);
        return NULL;
    }
}