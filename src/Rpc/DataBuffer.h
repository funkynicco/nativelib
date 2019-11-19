#pragma once

namespace nl
{
    namespace rpc
    {
        class DataBuffer
        {
        public:
            DataBuffer();
            ~DataBuffer();

            // no copying..
            DataBuffer(const DataBuffer&) = delete;
            DataBuffer& operator =(const DataBuffer&) = delete;

            DataBuffer(DataBuffer&& buffer) noexcept;
            DataBuffer& operator =(DataBuffer&& buffer) noexcept;

            size_t GetOffset() const { return m_offset; }
            size_t GetLength() const { return m_length; }
            size_t GetCapacity() const { return m_capacity; }

            void* GetData() { return m_buffer; }
            const void* GetData() const { return m_buffer; }

            void SetOffset(size_t offset) { m_offset = offset; }
            void SetLength(size_t length) { m_length = length; }

            DataBuffer& Read(void* lp, size_t length);
            DataBuffer& Write(const void* lp, size_t length);
            std::string ReadString();
            DataBuffer& WriteString(const std::string& str);
            
            DataBuffer& Delete(size_t count);

            template <typename T>
            DataBuffer& operator >>(T& value) { return Read(&value, sizeof(T)); }

            template <typename T>
            DataBuffer& operator <<(const T& value) { return Write(&value, sizeof(T)); }

        private:
            char* m_buffer;
            size_t m_offset;
            size_t m_length;
            size_t m_capacity;

            void EnsureWrite(size_t add);
        };
    }
}