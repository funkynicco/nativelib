/*
 * JSON Library by Nicco © 2019
 */

#pragma once

#include <NativeLib/Exceptions.h>
#include <NativeLib/Allocators.h>
#include <NativeLib/String.h>
#include <NativeLib/Containers/Vector.h>
#include <NativeLib/RAII/Shared.h>
#include <NativeLib/Containers/Map.h>

#include <stdint.h>
#include <type_traits>

namespace nl
{
    enum class JsonType
    {
        Null,
        String,
        Number,
        Object,
        Array,
        Boolean
    };

    class JsonNull;
    class JsonObject;
    class JsonArray;
    class JsonBoolean;
    class JsonString;
    class JsonNumber;

    class IncorrectMemberTypeException : public Exception
    {
    public:
        IncorrectMemberTypeException() :
            Exception("The JSON member is not of this type.")
        {
        }
    };

    class UnsupportedJsonTypeException : public Exception
    {
    public:
        UnsupportedJsonTypeException() :
            Exception("The requested JSON type is not supported in this method.")
        {
        }
    };

    ////////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////

    class JsonBase
    {
    public:
        virtual ~JsonBase() {}
        //DeclarePool(JsonBase);

        JsonType GetType() const { return m_type; }

    protected:
        friend Shared<JsonBase> Json_ReadValue(const nl::String& json, size_t& i, nl::Vector<nl::String>& parse_errors);
        JsonBase(JsonType type);
        JsonType m_type;
    };

    ////////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////

    class JsonNull : public JsonBase
    {
    public:
        JsonNull() :
            JsonBase(JsonType::Null)
        {
        }

        virtual ~JsonNull() {}

        //DeclarePool(JsonNull);

        static constexpr bool IsOfType(JsonType type) { return type == JsonType::Null; }
    };

    ////////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////

    class JsonObject : public JsonBase
    {
    public:
        JsonObject() :
            JsonBase(JsonType::Object)
        {
        }

        virtual ~JsonObject();
        //DeclarePool(JsonObject);

        static constexpr bool IsOfType(JsonType type) { return type == JsonType::Object; }

        Shared<JsonBase> GetMember(const char* name)
        {
            auto it = m_members.find(name);
            return it != m_members.end() ? it->second : nullptr;
        }

        Shared<const JsonBase> GetMember(const char* name) const
        {
            auto it = m_members.find(name);
            return it != m_members.end() ? it->second : nullptr;
        }

        template <typename T>
        Shared<T> GetMember(const char* name)
        {
            auto pMember = GetMember(name);
            if (!pMember)
                return nullptr;

            if (!T::IsOfType(pMember->GetType()))
                throw IncorrectMemberTypeException();

            return Shared<T>::Cast(pMember);
        }

        template <typename T>
        Shared<const T> GetMember(const char* name) const
        {
            return Shared<const T>::Cast(GetMember(name));
        }

        size_t GetCount() const { return m_members.GetCount(); }
        nl::Map<nl::String, Shared<JsonBase>>& GetMembers() { return m_members; }
        const nl::Map<nl::String, Shared<JsonBase>>& GetMembers() const { return m_members; }

        void SetNull(const char* pszName);
        void SetObject(const char* pszName, Shared<JsonBase> obj);
        Shared<JsonObject> SetObject(const char* pszName);
        Shared<JsonArray> SetArray(const char* pszName);
        Shared<JsonBoolean> SetBoolean(const char* pszName, bool value);
        Shared<JsonString> SetString(const char* pszName, const char* value);
        Shared<JsonNumber> SetNumber(const char* pszName, int64_t value);
        Shared<JsonNumber> SetNumber(const char* pszName, double value);

        Shared<JsonNumber> SetNumber(const char* pszName, float value) { return SetNumber(pszName, (double)value); }
        Shared<JsonNumber> SetNumber(const char* pszName, char value) { return SetNumber(pszName, (int64_t)value); }
        Shared<JsonNumber> SetNumber(const char* pszName, uint8_t value) { return SetNumber(pszName, (int64_t)value); }
        Shared<JsonNumber> SetNumber(const char* pszName, int16_t value) { return SetNumber(pszName, (int64_t)value); }
        Shared<JsonNumber> SetNumber(const char* pszName, int32_t value) { return SetNumber(pszName, (int64_t)value); }
        Shared<JsonNumber> SetNumber(const char* pszName, uint32_t value) { return SetNumber(pszName, (int64_t)value); }

    protected:
        friend Shared<JsonBase> Json_ReadValue(const nl::String& json, size_t& i, nl::Vector<nl::String>& parse_errors);
        friend Shared<JsonBase> ParseJson(const char* pszJson, nl::Vector<nl::String>& parse_errors);

        bool Read(const nl::String& json, size_t& i, nl::Vector<nl::String>& parse_errors);

    private:
        template <typename T>
        void SetBase(const char* pszName, Shared<T> pItem)
        {
            auto it = m_members.find(pszName);
            if (it != m_members.end())
                m_members.Remove(it);

            m_members.Add(pszName, pItem);
        }

        nl::Map<nl::String, Shared<JsonBase>> m_members;
    };

    ////////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////

    class JsonArray : public JsonBase
    {
    public:
        JsonArray() :
            JsonBase(JsonType::Array)
        {
        }

        virtual ~JsonArray();
        //DeclarePool(JsonArray);

        static constexpr bool IsOfType(JsonType type) { return type == JsonType::Array; }

        size_t GetCount() const { return m_items.GetCount(); }
        Shared<JsonBase> GetItem(size_t index) { return m_items[index]; }
        Shared<const JsonBase> GetItem(size_t index) const { return m_items[index]; }

        template <typename T>
        Shared<T> GetItem(size_t index)
        {
            return Shared<T>::Cast(m_items[index]);
        }

        template <typename T>
        Shared<const T> GetItem(size_t index) const
        {
            return Shared<const T>::Cast(m_items[index]);
        }

        void AddNull();
        void AddObject(Shared<JsonBase> obj);
        Shared<JsonObject> AddObject();
        Shared<JsonArray> AddArray();
        Shared<JsonBoolean> AddBoolean(bool value);
        Shared<JsonString> AddString(const char* value);
        Shared<JsonNumber> AddNumber(int64_t value);
        Shared<JsonNumber> AddNumber(double value);

        Shared<JsonNumber> AddNumber(float value) { return AddNumber((double)value); }
        Shared<JsonNumber> AddNumber(char value) { return AddNumber((int64_t)value); }
        Shared<JsonNumber> AddNumber(uint8_t value) { return AddNumber((int64_t)value); }
        Shared<JsonNumber> AddNumber(int16_t value) { return AddNumber((int64_t)value); }
        Shared<JsonNumber> AddNumber(int32_t value) { return AddNumber((int64_t)value); }
        Shared<JsonNumber> AddNumber(uint32_t value) { return AddNumber((int64_t)value); }

    protected:
        friend Shared<JsonBase> Json_ReadValue(const nl::String& json, size_t& i, nl::Vector<nl::String>& parse_errors);
        friend Shared<JsonBase> ParseJson(const char* pszJson, nl::Vector<nl::String>& parse_errors);

        bool Read(const nl::String& json, size_t& i, nl::Vector<nl::String>& parse_errors);

    private:
        nl::Vector<Shared<JsonBase>> m_items;

        template <typename T>
        void AddBase(Shared<T> pItem)
        {
            m_items.Add(pItem);
        }
    };

    ////////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////

    class JsonString : public JsonBase
    {
    public:
        JsonString() :
            JsonBase(JsonType::String)
        {
        }

        JsonString(const nl::String& value) :
            JsonBase(JsonType::String)
        {
            m_value = value;
        }

        virtual ~JsonString() {}
        //DeclarePool(JsonString);

        static constexpr bool IsOfType(JsonType type) { return type == JsonType::String; }

        const nl::String& GetValue() const { return m_value; }
        void SetValue(const nl::String& value) { m_value = value; }

    protected:
        friend Shared<JsonBase> Json_ReadValue(const nl::String& json, size_t& i, nl::Vector<nl::String>& parse_errors);

    private:
        nl::String m_value;
    };

    ////////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////

    class JsonNumber : public JsonBase
    {
    public:
        JsonNumber() :
            JsonBase(JsonType::Number)
        {
            m_bIsDouble = false;
            m_value = 0;
            m_double = 0;
        }

        JsonNumber(int64_t value) :
            JsonBase(JsonType::Number)
        {
            m_bIsDouble = false;
            m_value = value;
            m_double = (double)value;
        }

        JsonNumber(double value) :
            JsonBase(JsonType::Number)
        {
            m_bIsDouble = true;
            m_value = (int64_t)value;
            m_double = value;
        }

        virtual ~JsonNumber() {}
        //DeclarePool(JsonNumber);

        static constexpr bool IsOfType(JsonType type) { return type == JsonType::Number; }

        int64_t GetValue() const { return m_value; }
        double GetDouble() const { return m_double; }

        template <typename T>
        T GetValue() { return (T)m_value; }

        template <typename T>
        const T GetValue() const { return (T)m_value; }

        bool IsDouble() const { return m_bIsDouble; }

        void SetValue(int64_t value)
        {
            m_bIsDouble = false;
            m_value = value;
            m_double = 0.0;
        }

        void SetValue(double value)
        {
            m_bIsDouble = true;
            m_value = 0;
            m_double = value;
        }

    protected:
        friend Shared<JsonBase> Json_ReadValue(const nl::String& json, size_t& i, nl::Vector<nl::String>& parse_errors);

    private:
        bool m_bIsDouble;
        int64_t m_value;
        double m_double;
    };

    ////////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////

    class JsonBoolean : public JsonBase
    {
    public:
        JsonBoolean() :
            JsonBase(JsonType::Boolean)
        {
            m_value = false;
        }

        JsonBoolean(bool value) :
            JsonBase(JsonType::Boolean)
        {
            m_value = value;
        }

        virtual ~JsonBoolean() {}
        //DeclarePool(JsonBoolean);

        static constexpr bool IsOfType(JsonType type) { return type == JsonType::Boolean; }

        bool GetValue() const { return m_value; }
        void SetValue(bool value) { m_value = value; }

    protected:
        friend Shared<JsonBase> Json_ReadValue(const nl::String& json, size_t& i, nl::Vector<nl::String>& parse_errors);

    private:
        bool m_value;
    };

    ////////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////

    Shared<JsonBase> ParseJson(const char* pszJson, nl::Vector<nl::String>& parse_errors);

    template <typename T>
    inline Shared<T> ParseJson(const char* pszJson, nl::Vector<nl::String>& parse_errors)
    {
        auto ptr = ParseJson(pszJson, parse_errors);
        if (!ptr)
            return nullptr;

        return Shared<T>::Cast(ptr);
    }

    bool GenerateJsonString(nl::String& output, Shared<const JsonBase> pJson);
    Shared<JsonBase> CreateJsonObject(JsonType type);

    template <typename T>
    inline Shared<T> CreateJsonObject()
    {
        Shared<JsonBase> ptr;

        if constexpr (std::is_same_v<T, JsonNull>)
            ptr = CreateJsonObject(JsonType::Null);
        else if constexpr (std::is_same_v<T, JsonObject>)
            ptr = CreateJsonObject(JsonType::Object);
        else if constexpr (std::is_same_v<T, JsonArray>)
            ptr = CreateJsonObject(JsonType::Array);
        else if constexpr (std::is_same_v<T, JsonString>)
            ptr = CreateJsonObject(JsonType::String);
        else if constexpr (std::is_same_v<T, JsonNumber>)
            ptr = CreateJsonObject(JsonType::Number);
        else if constexpr (std::is_same_v<T, JsonBoolean>)
            ptr = CreateJsonObject(JsonType::Boolean);
        else
            throw UnsupportedJsonTypeException();

        return Shared<T>::Cast(ptr);
    }
}