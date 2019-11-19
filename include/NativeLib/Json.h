/*
 * JSON Library by Nicco © 2019
 */

#pragma once

#include <NativeLib/Exceptions.h>

#include <unordered_map>
#include <vector>
#include <string>
#include <type_traits>
#include <memory>

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

        inline JsonType GetType() const { return m_type; }

    protected:
        friend JsonBase* Json_ReadValue(const std::string& json, size_t& i, std::vector<std::string>& parse_errors);
        JsonBase(JsonType type);
        JsonType m_type;

    private:
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

        inline JsonBase* GetMember(const char* name)
        {
            std::unordered_map<std::string, JsonBase*>::iterator it = m_members.find(name);
            return it != m_members.end() ? it->second : NULL;
        }

        inline const JsonBase* GetMember(const char* name) const
        {
            std::unordered_map<std::string, JsonBase*>::const_iterator it = m_members.find(name);
            return it != m_members.end() ? it->second : NULL;
        }

        template <typename T>
        inline T* GetMember(const char* name)
        {
            auto pMember = GetMember(name);
            if (!pMember)
                return nullptr;

            if (!T::IsOfType(pMember->GetType()))
                throw IncorrectMemberTypeException();

            return static_cast<T*>(pMember);
        }

        template <typename T>
        inline const T* GetMember(const char* name) const { return static_cast<const T*>(GetMember(name)); }

        inline size_t GetCount() const { return m_members.size(); }
        inline std::unordered_map<std::string, JsonBase*>& GetMembers() { return m_members; }
        inline const std::unordered_map<std::string, JsonBase*>& GetMembers() const { return m_members; }

        void            SetNull(const char* pszName);
        void            SetObject(const char* pszName, JsonBase* obj);
        JsonObject* SetObject(const char* pszName);
        JsonArray* SetArray(const char* pszName);
        JsonBoolean* SetBoolean(const char* pszName, bool value);
        JsonString* SetString(const char* pszName, const char* value);
        JsonNumber* SetNumber(const char* pszName, long long value);
        JsonNumber* SetNumber(const char* pszName, double value);

        inline JsonNumber* SetNumber(const char* pszName, float value) { return SetNumber(pszName, (double)value); }
        inline JsonNumber* SetNumber(const char* pszName, char value) { return SetNumber(pszName, (long long)value); }
        inline JsonNumber* SetNumber(const char* pszName, unsigned char value) { return SetNumber(pszName, (long long)value); }
        inline JsonNumber* SetNumber(const char* pszName, short value) { return SetNumber(pszName, (long long)value); }
        inline JsonNumber* SetNumber(const char* pszName, int value) { return SetNumber(pszName, (long long)value); }
        inline JsonNumber* SetNumber(const char* pszName, unsigned long value) { return SetNumber(pszName, (long long)value); }

    protected:
        friend JsonBase* Json_ReadValue(const std::string& json, size_t& i, std::vector<std::string>& parse_errors);
        friend std::unique_ptr<JsonBase> ParseJson(const char* pszJson, std::vector<std::string>& parse_errors);

        bool Read(const std::string& json, size_t& i, std::vector<std::string>& parse_errors);

    private:

        template <typename T>
        inline void SetBase(const char* pszName, T* pItem)
        {
            auto it = m_members.find(pszName);
            if (it != m_members.end())
            {
                delete it->second;
                m_members.erase(it);
            }

            m_members.insert(std::pair<std::string, JsonBase*>(pszName, pItem));
        }

        std::unordered_map<std::string, JsonBase*> m_members;
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

        inline size_t GetCount() const { return m_items.size(); }
        inline JsonBase* GetItem(size_t index) { return m_items[index]; }
        inline const JsonBase* GetItem(size_t index) const { return m_items[index]; }

        template <typename T>
        inline T* GetItem(size_t index) { return static_cast<T*>(m_items[index]); }

        template <typename T>
        inline const T* GetItem(size_t index) const { return static_cast<T*>(m_items[index]); }

        void AddNull();
        void AddObject(JsonBase* obj);
        JsonObject* AddObject();
        JsonArray* AddArray();
        JsonBoolean* AddBoolean(bool value);
        JsonString* AddString(const char* value);
        JsonNumber* AddNumber(long long value);
        JsonNumber* AddNumber(double value);

        inline JsonNumber* AddNumber(float value) { return AddNumber((double)value); }
        inline JsonNumber* AddNumber(char value) { return AddNumber((long long)value); }
        inline JsonNumber* AddNumber(unsigned char value) { return AddNumber((long long)value); }
        inline JsonNumber* AddNumber(short value) { return AddNumber((long long)value); }
        inline JsonNumber* AddNumber(int value) { return AddNumber((long long)value); }
        inline JsonNumber* AddNumber(unsigned long value) { return AddNumber((long long)value); }

    protected:
        friend JsonBase* Json_ReadValue(const std::string& json, size_t& i, std::vector<std::string>& parse_errors);
        friend std::unique_ptr<JsonBase> ParseJson(const char* pszJson, std::vector<std::string>& parse_errors);

        bool Read(const std::string& json, size_t& i, std::vector<std::string>& parse_errors);

    private:
        std::vector<JsonBase*> m_items;

        template <typename T>
        inline void AddBase(T* pItem)
        {
            m_items.push_back(pItem);
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

        JsonString(const std::string& value) :
            JsonBase(JsonType::String)
        {
            m_value = value;
        }

        virtual ~JsonString() {}
        //DeclarePool(JsonString);

        static constexpr bool IsOfType(JsonType type) { return type == JsonType::String; }

        inline const std::string& GetValue() const { return m_value; }
        inline void SetValue(const std::string& value) { m_value = value; }

    protected:
        friend JsonBase* Json_ReadValue(const std::string& json, size_t& i, std::vector<std::string>& parse_errors);

    private:
        std::string m_value;
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

        JsonNumber(__int64 value) :
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
            m_value = (__int64)value;
            m_double = value;
        }

        virtual ~JsonNumber() {}
        //DeclarePool(JsonNumber);

        static constexpr bool IsOfType(JsonType type) { return type == JsonType::Number; }

        inline __int64 GetValue() const { return m_value; }
        inline double GetDouble() const { return m_double; }

        template <typename T>
        inline T GetValue() { return (T)m_value; }

        template <typename T>
        inline const T GetValue() const { return (T)m_value; }

        inline bool IsDouble() const { return m_bIsDouble; }

        inline void SetValue(__int64 value)
        {
            m_bIsDouble = false;
            m_value = value;
            m_double = 0.0;
        }

        inline void SetValue(double value)
        {
            m_bIsDouble = true;
            m_value = 0;
            m_double = value;
        }

    protected:
        friend JsonBase* Json_ReadValue(const std::string& json, size_t& i, std::vector<std::string>& parse_errors);

    private:
        bool m_bIsDouble;
        __int64 m_value;
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

        inline bool GetValue() const { return m_value; }
        inline void SetValue(bool value) { m_value = value; }

    protected:
        friend JsonBase* Json_ReadValue(const std::string& json, size_t& i, std::vector<std::string>& parse_errors);

    private:
        bool m_value;
    };

    ////////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////

    std::unique_ptr<JsonBase> ParseJson(const char* pszJson, std::vector<std::string>& parse_errors);

    template <typename T>
    inline std::unique_ptr<T> ParseJson(const char* pszJson, std::vector<std::string>& parse_errors)
    {
        auto ptr = ParseJson(pszJson, parse_errors);
        if (!ptr)
            return nullptr;

        return std::unique_ptr<T>(static_cast<T*>(ptr.release()));
    }

    bool GenerateJsonString(std::string& output, const JsonBase* pJson);
    std::unique_ptr<JsonBase> CreateJsonObject(JsonType type);

    template <typename T>
    inline std::unique_ptr<T> CreateJsonObject()
    {
        std::unique_ptr<JsonBase> ptr;

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

        return std::unique_ptr<T>(static_cast<T*>(ptr.release()));
    }
}