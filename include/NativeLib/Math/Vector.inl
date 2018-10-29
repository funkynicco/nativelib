#pragma once

namespace nl
{
    namespace math
    {
        /***************************************************************************************
         * Vector2
         ***************************************************************************************/

        inline bool __vectorcall Vector2::operator ==(Vector2 other)
        {
            return
                x == other.x &&
                y == other.y;
        }

        inline bool __vectorcall Vector2::operator !=(Vector2 other)
        {
            return
                x != other.x ||
                y != other.y;
        }

        inline Vector2& __vectorcall Vector2::operator +=(Vector2 other)
        {
            x += other.x;
            y += other.y;
            return *this;
        }

        inline Vector2& __vectorcall Vector2::operator -=(Vector2 other)
        {
            x -= other.x;
            y -= other.y;
            return *this;
        }

        inline Vector2& __vectorcall Vector2::operator *=(Vector2 other)
        {
            x *= other.x;
            y *= other.y;
            return *this;
        }

        inline Vector2& __vectorcall Vector2::operator /=(Vector2 other)
        {
            x /= other.x;
            y /= other.y;
            return *this;
        }

        inline Vector2& __vectorcall Vector2::operator +=(float value)
        {
            x += value;
            y += value;
            return *this;
        }

        inline Vector2& __vectorcall Vector2::operator -=(float value)
        {
            x -= value;
            y -= value;
            return *this;
        }

        inline Vector2& __vectorcall Vector2::operator *=(float value)
        {
            x *= value;
            y *= value;
            return *this;
        }

        inline Vector2& __vectorcall Vector2::operator /=(float value)
        {
            x /= value;
            y /= value;
            return *this;
        }

        inline Vector2 __vectorcall Vector2::operator +(Vector2 other) const
        {
            return Vector2(
                x + other.x,
                y + other.y);
        }

        inline Vector2 __vectorcall Vector2::operator -(Vector2 other) const
        {
            return Vector2(
                x - other.x,
                y - other.y);
        }

        inline Vector2 __vectorcall Vector2::operator *(Vector2 other) const
        {
            return Vector2(
                x * other.x,
                y * other.y);
        }

        inline Vector2 __vectorcall Vector2::operator /(Vector2 other) const
        {
            return Vector2(
                x / other.x,
                y / other.y);
        }

        inline Vector2 __vectorcall Vector2::operator +(float value) const
        {
            return Vector2(
                x + value,
                y + value);
        }

        inline Vector2 __vectorcall Vector2::operator -(float value) const
        {
            return Vector2(
                x - value,
                y - value);
        }

        inline Vector2 __vectorcall Vector2::operator *(float value) const
        {
            return Vector2(
                x * value,
                y * value);
        }

        inline Vector2 __vectorcall Vector2::operator /(float value) const
        {
            return Vector2(
                x / value,
                y / value);
        }

        inline void Vector2::Normalize()
        {
            float length = sqrtf(x * x + y * y);
            x /= length;
            y /= length;
        }

        inline Vector2 __vectorcall Vector2::Normalize(Vector2 vec)
        {
            float length = sqrtf(vec.x * vec.x + vec.y * vec.y);
            return Vector2(
                vec.x / length,
                vec.y / length);
        }

        inline Vector2 __vectorcall Vector2::Minimize(Vector2 v1, Vector2 v2)
        {
            return Vector2(
                v1.x < v2.x ? v1.x : v2.x,
                v1.y < v2.y ? v1.y : v2.y);
        }

        inline Vector2 __vectorcall Vector2::Maximize(Vector2 v1, Vector2 v2)
        {
            return Vector2(
                v1.x > v2.x ? v1.x : v2.x,
                v1.y > v2.y ? v1.y : v2.y);
        }

        inline Vector2 __vectorcall Vector2::Clamp(Vector2 value, Vector2 min, Vector2 max)
        {
            return Vector2(
                value.x < min.x ? min.x : value.x > max.x ? max.x : value.x,
                value.y < min.y ? min.y : value.y > max.y ? max.y : value.y);
        }

        inline Vector2 __vectorcall Vector2::Lerp(Vector2 min, Vector2 max, float value)
        {
            return Vector2(
                (max.x - min.x) * value + min.x,
                (max.y - min.y) * value + min.y);
        }

        inline float __vectorcall Vector2::Length(Vector2 vec)
        {
            return sqrtf(vec.x * vec.x + vec.y * vec.y);
        }

        inline float __vectorcall Vector2::Dot(Vector2 v1, Vector2 v2)
        {
            return v1.x * v2.x + v1.y * v2.y;
        }

        /***************************************************************************************
         * Vector3
         ***************************************************************************************/

        inline bool __vectorcall Vector3::operator ==(Vector3 other)
        {
            return
                x == other.x &&
                y == other.y &&
                z == other.z;
        }

        inline bool __vectorcall Vector3::operator !=(Vector3 other)
        {
            return
                x != other.x ||
                y != other.y ||
                z != other.z;
        }

        inline Vector3& __vectorcall Vector3::operator +=(Vector3 other)
        {
            x += other.x;
            y += other.y;
            z += other.z;
            return *this;
        }

        inline Vector3& __vectorcall Vector3::operator -=(Vector3 other)
        {
            x -= other.x;
            y -= other.y;
            z -= other.z;
            return *this;
        }

        inline Vector3& __vectorcall Vector3::operator *=(Vector3 other)
        {
            x *= other.x;
            y *= other.y;
            z *= other.z;
            return *this;
        }

        inline Vector3& __vectorcall Vector3::operator /=(Vector3 other)
        {
            x /= other.x;
            y /= other.y;
            z /= other.z;
            return *this;
        }

        inline Vector3& __vectorcall Vector3::operator +=(float value)
        {
            x += value;
            y += value;
            z += value;
            return *this;
        }

        inline Vector3& __vectorcall Vector3::operator -=(float value)
        {
            x -= value;
            y -= value;
            z -= value;
            return *this;
        }

        inline Vector3& __vectorcall Vector3::operator *=(float value)
        {
            x *= value;
            y *= value;
            z *= value;
            return *this;
        }

        inline Vector3& __vectorcall Vector3::operator /=(float value)
        {
            x /= value;
            y /= value;
            z /= value;
            return *this;
        }

        inline Vector3 __vectorcall Vector3::operator +(Vector3 other) const
        {
            return Vector3(
                x + other.x,
                y + other.y,
                z + other.z);
        }

        inline Vector3 __vectorcall Vector3::operator -(Vector3 other) const
        {
            return Vector3(
                x - other.x,
                y - other.y,
                z - other.z);
        }

        inline Vector3 __vectorcall Vector3::operator *(Vector3 other) const
        {
            return Vector3(
                x * other.x,
                y * other.y,
                z * other.z);
        }

        inline Vector3 __vectorcall Vector3::operator /(Vector3 other) const
        {
            return Vector3(
                x / other.x,
                y / other.y,
                z / other.z);
        }

        inline Vector3 __vectorcall Vector3::operator +(float value) const
        {
            return Vector3(
                x + value,
                y + value,
                z + value);
        }

        inline Vector3 __vectorcall Vector3::operator -(float value) const
        {
            return Vector3(
                x - value,
                y - value,
                z - value);
        }

        inline Vector3 __vectorcall Vector3::operator *(float value) const
        {
            return Vector3(
                x * value,
                y * value,
                z * value);
        }

        inline Vector3 __vectorcall Vector3::operator /(float value) const
        {
            return Vector3(
                x / value,
                y / value,
                z / value);
        }

        inline void Vector3::Normalize()
        {
            float length = sqrtf(x * x + y * y + z * z);
            x /= length;
            y /= length;
            z /= length;
        }

        inline Vector3 __vectorcall Vector3::Normalize(Vector3 vec)
        {
            float length = sqrtf(vec.x * vec.x + vec.y * vec.y + vec.z * vec.z);
            return Vector3(
                vec.x / length,
                vec.y / length,
                vec.z / length);
        }

        inline Vector3 __vectorcall Vector3::Minimize(Vector3 v1, Vector3 v2)
        {
            return Vector3(
                v1.x < v2.x ? v1.x : v2.x,
                v1.y < v2.y ? v1.y : v2.y,
                v1.z < v2.z ? v1.z : v2.z);
        }

        inline Vector3 __vectorcall Vector3::Maximize(Vector3 v1, Vector3 v2)
        {
            return Vector3(
                v1.x > v2.x ? v1.x : v2.x,
                v1.y > v2.y ? v1.y : v2.y,
                v1.z > v2.z ? v1.z : v2.z);
        }

        inline Vector3 __vectorcall Vector3::Clamp(Vector3 value, Vector3 min, Vector3 max)
        {
            return Vector3(
                value.x < min.x ? min.x : value.x > max.x ? max.x : value.x,
                value.y < min.y ? min.y : value.y > max.y ? max.y : value.y,
                value.z < min.z ? min.z : value.z > max.z ? max.z : value.z);
        }

        inline Vector3 __vectorcall Vector3::Lerp(Vector3 min, Vector3 max, float value)
        {
            return Vector3(
                (max.x - min.x) * value + min.x,
                (max.y - min.y) * value + min.y,
                (max.z - min.z) * value + min.z);
        }

        inline float __vectorcall Vector3::Length(Vector3 vec)
        {
            return sqrtf(vec.x * vec.x + vec.y * vec.y + vec.z * vec.z);
        }

        inline float __vectorcall Vector3::Dot(Vector3 v1, Vector3 v2)
        {
            return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
        }

        inline Vector3 __vectorcall Vector3::Cross(Vector3 v1, Vector3 v2)
        {
            return Vector3(
                v1.y * v2.z - v1.z * v2.y,
                v1.z * v2.x - v1.x * v2.z,
                v1.x * v2.y - v1.y * v2.x);
        }

        /***************************************************************************************
         * Vector4
         ***************************************************************************************/

        inline bool __vectorcall Vector4::operator ==(Vector4 other)
        {
            return
                x == other.x &&
                y == other.y &&
                z == other.z &&
                w == other.w;
        }

        inline bool __vectorcall Vector4::operator !=(Vector4 other)
        {
            return
                x != other.x ||
                y != other.y ||
                z != other.z ||
                w != other.w;
        }

        inline Vector4& __vectorcall Vector4::operator +=(Vector4 other)
        {
            x += other.x;
            y += other.y;
            z += other.z;
            w += other.w;
            return *this;
        }

        inline Vector4& __vectorcall Vector4::operator -=(Vector4 other)
        {
            x -= other.x;
            y -= other.y;
            z -= other.z;
            w -= other.w;
            return *this;
        }

        inline Vector4& __vectorcall Vector4::operator *=(Vector4 other)
        {
            x *= other.x;
            y *= other.y;
            z *= other.z;
            w *= other.w;
            return *this;
        }

        inline Vector4& __vectorcall Vector4::operator /=(Vector4 other)
        {
            x /= other.x;
            y /= other.y;
            z /= other.z;
            w /= other.w;
            return *this;
        }

        inline Vector4& __vectorcall Vector4::operator +=(float value)
        {
            x += value;
            y += value;
            z += value;
            w += value;
            return *this;
        }

        inline Vector4& __vectorcall Vector4::operator -=(float value)
        {
            x -= value;
            y -= value;
            z -= value;
            w -= value;
            return *this;
        }

        inline Vector4& __vectorcall Vector4::operator *=(float value)
        {
            x *= value;
            y *= value;
            z *= value;
            w *= value;
            return *this;
        }

        inline Vector4& __vectorcall Vector4::operator /=(float value)
        {
            x /= value;
            y /= value;
            z /= value;
            w /= value;
            return *this;
        }

        inline Vector4 __vectorcall Vector4::operator +(Vector4 other) const
        {
            return Vector4(
                x + other.x,
                y + other.y,
                z + other.z,
                w + other.w);
        }

        inline Vector4 __vectorcall Vector4::operator -(Vector4 other) const
        {
            return Vector4(
                x - other.x,
                y - other.y,
                z - other.z,
                w - other.w);
        }

        inline Vector4 __vectorcall Vector4::operator *(Vector4 other) const
        {
            return Vector4(
                x * other.x,
                y * other.y,
                z * other.z,
                w * other.w);
        }

        inline Vector4 __vectorcall Vector4::operator /(Vector4 other) const
        {
            return Vector4(
                x / other.x,
                y / other.y,
                z / other.z,
                w / other.w);
        }

        inline Vector4 __vectorcall Vector4::operator +(float value) const
        {
            return Vector4(
                x + value,
                y + value,
                z + value,
                w + value);
        }

        inline Vector4 __vectorcall Vector4::operator -(float value) const
        {
            return Vector4(
                x - value,
                y - value,
                z - value,
                w - value);
        }

        inline Vector4 __vectorcall Vector4::operator *(float value) const
        {
            return Vector4(
                x * value,
                y * value,
                z * value,
                w * value);
        }

        inline Vector4 __vectorcall Vector4::operator /(float value) const
        {
            return Vector4(
                x / value,
                y / value,
                z / value,
                w / value);
        }

        inline void Vector4::Normalize()
        {
            float length = sqrtf(x * x + y * y + z * z + w * w);
            x /= length;
            y /= length;
            z /= length;
            w /= length;
        }

        inline Vector4 __vectorcall Vector4::Normalize(Vector4 vec)
        {
            float length = sqrtf(vec.x * vec.x + vec.y * vec.y + vec.z * vec.z + vec.w * vec.w);
            return Vector4(
                vec.x / length,
                vec.y / length,
                vec.z / length,
                vec.w / length);
        }

        inline Vector4 __vectorcall Vector4::Minimize(Vector4 v1, Vector4 v2)
        {
            return Vector4(
                v1.x < v2.x ? v1.x : v2.x,
                v1.y < v2.y ? v1.y : v2.y,
                v1.z < v2.z ? v1.z : v2.z,
                v1.w < v2.w ? v1.w : v2.w);
        }

        inline Vector4 __vectorcall Vector4::Maximize(Vector4 v1, Vector4 v2)
        {
            return Vector4(
                v1.x > v2.x ? v1.x : v2.x,
                v1.y > v2.y ? v1.y : v2.y,
                v1.z > v2.z ? v1.z : v2.z,
                v1.w > v2.w ? v1.w : v2.w);
        }

        inline Vector4 __vectorcall Vector4::Clamp(Vector4 value, Vector4 min, Vector4 max)
        {
            return Vector4(
                value.x < min.x ? min.x : value.x > max.x ? max.x : value.x,
                value.y < min.y ? min.y : value.y > max.y ? max.y : value.y,
                value.z < min.z ? min.z : value.z > max.z ? max.z : value.z,
                value.w < min.w ? min.w : value.w > max.w ? max.w : value.w);
        }

        inline Vector4 __vectorcall Vector4::Lerp(Vector4 min, Vector4 max, float value)
        {
            return Vector4(
                (max.x - min.x) * value + min.x,
                (max.y - min.y) * value + min.y,
                (max.z - min.z) * value + min.z,
                (max.w - min.w) * value + min.w);
        }

        inline float __vectorcall Vector4::Length(Vector4 vec)
        {
            return sqrtf(vec.x * vec.x + vec.y * vec.y + vec.z * vec.z + vec.w * vec.w);
        }

        inline float __vectorcall Vector4::Dot(Vector4 v1, Vector4 v2)
        {
            return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z + v1.w * v2.w;
        }

        inline bool __vectorcall Vector4::Intersects(Vector4 container, Vector4 rc)
        {
            return
                rc.x <= container.x + container.z &&
                rc.y <= container.y + container.w &&
                rc.x + rc.z >= container.x &&
                rc.y + rc.w >= container.y;
        }

        inline bool __vectorcall Vector4::Contains(Vector4 container, Vector2 pt)
        {
            return
                pt.x >= container.x &&
                pt.y >= container.y &&
                pt.x <= container.x + container.z &&
                pt.y <= container.y + container.w;
        }

        inline bool __vectorcall Vector4::Contains(Vector4 container, Vector4 rc)
        {
            return
                rc.x >= container.x &&
                rc.y >= container.y &&
                rc.x + rc.z <= container.x + container.z &&
                rc.y + rc.w <= container.y + container.w;
        }
    }
}