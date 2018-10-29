#pragma once

#include <math.h>

namespace nl
{
    namespace math
    {
        struct Vector2
        {
            float x, y;

            inline Vector2() { x = y = 0; }

            inline Vector2(float x, float y)
            {
                this->x = x;
                this->y = y;
            }

            bool __vectorcall operator ==(Vector2 other);
            bool __vectorcall operator !=(Vector2 other);

            Vector2& __vectorcall operator +=(Vector2 other);
            Vector2& __vectorcall operator -=(Vector2 other);
            Vector2& __vectorcall operator *=(Vector2 other);
            Vector2& __vectorcall operator /=(Vector2 other);
            Vector2& __vectorcall operator +=(float value);
            Vector2& __vectorcall operator -=(float value);
            Vector2& __vectorcall operator *=(float value);
            Vector2& __vectorcall operator /=(float value);

            Vector2 __vectorcall operator +(Vector2 other) const;
            Vector2 __vectorcall operator -(Vector2 other) const;
            Vector2 __vectorcall operator *(Vector2 other) const;
            Vector2 __vectorcall operator /(Vector2 other) const;
            Vector2 __vectorcall operator +(float value) const;
            Vector2 __vectorcall operator -(float value) const;
            Vector2 __vectorcall operator *(float value) const;
            Vector2 __vectorcall operator /(float value) const;

            void Normalize();

            static Vector2 __vectorcall Normalize(Vector2 vec);
            static Vector2 __vectorcall Minimize(Vector2 v1, Vector2 v2);
            static Vector2 __vectorcall Maximize(Vector2 v1, Vector2 v2);
            static Vector2 __vectorcall Clamp(Vector2 value, Vector2 min, Vector2 max);
            static Vector2 __vectorcall Lerp(Vector2 min, Vector2 max, float value);
            static float __vectorcall Length(Vector2 vec);
            static float __vectorcall Dot(Vector2 v1, Vector2 v2);
        };

        struct Vector3
        {
            float x, y, z;

            inline Vector3() { x = y = z = 0; }

            inline Vector3(float x, float y, float z)
            {
                this->x = x;
                this->y = y;
                this->z = z;
            }

            inline Vector3(Vector2 xy, float z)
            {
                this->x = xy.x;
                this->y = xy.y;
                this->z = z;
            }

            bool __vectorcall operator ==(Vector3 other);
            bool __vectorcall operator !=(Vector3 other);

            Vector3& __vectorcall operator +=(Vector3 other);
            Vector3& __vectorcall operator -=(Vector3 other);
            Vector3& __vectorcall operator *=(Vector3 other);
            Vector3& __vectorcall operator /=(Vector3 other);
            Vector3& __vectorcall operator +=(float value);
            Vector3& __vectorcall operator -=(float value);
            Vector3& __vectorcall operator *=(float value);
            Vector3& __vectorcall operator /=(float value);

            Vector3 __vectorcall operator +(Vector3 other) const;
            Vector3 __vectorcall operator -(Vector3 other) const;
            Vector3 __vectorcall operator *(Vector3 other) const;
            Vector3 __vectorcall operator /(Vector3 other) const;
            Vector3 __vectorcall operator +(float value) const;
            Vector3 __vectorcall operator -(float value) const;
            Vector3 __vectorcall operator *(float value) const;
            Vector3 __vectorcall operator /(float value) const;

            void Normalize();

            static Vector3 __vectorcall Normalize(Vector3 vec);
            static Vector3 __vectorcall Minimize(Vector3 v1, Vector3 v2);
            static Vector3 __vectorcall Maximize(Vector3 v1, Vector3 v2);
            static Vector3 __vectorcall Clamp(Vector3 value, Vector3 min, Vector3 max);
            static Vector3 __vectorcall Lerp(Vector3 min, Vector3 max, float value);
            static float __vectorcall Length(Vector3 vec);
            static float __vectorcall Dot(Vector3 v1, Vector3 v2);
            static Vector3 __vectorcall Cross(Vector3 v1, Vector3 v2);
        };

        struct Vector4
        {
            float x, y, z, w;

            inline Vector4() { x = y = z = w = 0; }

            inline Vector4(float x, float y, float z, float w)
            {
                this->x = x;
                this->y = y;
                this->z = z;
                this->w = w;
            }

            inline Vector4(Vector3 xyz, float w)
            {
                this->x = xyz.x;
                this->y = xyz.y;
                this->z = xyz.z;
                this->w = w;
            }

            inline Vector4(Vector2 xy, Vector2 zw)
            {
                this->x = xy.x;
                this->y = xy.y;
                this->z = zw.x;
                this->w = zw.y;
            }

            bool __vectorcall operator ==(Vector4 other);
            bool __vectorcall operator !=(Vector4 other);

            Vector4& __vectorcall operator +=(Vector4 other);
            Vector4& __vectorcall operator -=(Vector4 other);
            Vector4& __vectorcall operator *=(Vector4 other);
            Vector4& __vectorcall operator /=(Vector4 other);
            Vector4& __vectorcall operator +=(float value);
            Vector4& __vectorcall operator -=(float value);
            Vector4& __vectorcall operator *=(float value);
            Vector4& __vectorcall operator /=(float value);

            Vector4 __vectorcall operator +(Vector4 other) const;
            Vector4 __vectorcall operator -(Vector4 other) const;
            Vector4 __vectorcall operator *(Vector4 other) const;
            Vector4 __vectorcall operator /(Vector4 other) const;
            Vector4 __vectorcall operator +(float value) const;
            Vector4 __vectorcall operator -(float value) const;
            Vector4 __vectorcall operator *(float value) const;
            Vector4 __vectorcall operator /(float value) const;

            void Normalize();

            static Vector4 __vectorcall Normalize(Vector4 vec);
            static Vector4 __vectorcall Minimize(Vector4 v1, Vector4 v2);
            static Vector4 __vectorcall Maximize(Vector4 v1, Vector4 v2);
            static Vector4 __vectorcall Clamp(Vector4 value, Vector4 min, Vector4 max);
            static Vector4 __vectorcall Lerp(Vector4 min, Vector4 max, float value);
            static float __vectorcall Length(Vector4 vec);
            static float __vectorcall Dot(Vector4 v1, Vector4 v2);

            // 2D
            static bool __vectorcall Intersects(Vector4 container, Vector4 rc);
            static bool __vectorcall Contains(Vector4 container, Vector2 pt);
            static bool __vectorcall Contains(Vector4 container, Vector4 rc);
        };
    }
}

#include "Vector.inl"