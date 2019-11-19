/*
 * JSON Library by Nicco © 2019
 */

#include "StdAfx.h"

#include <NativeLib/Json.h>

 //DefinePool(JsonBase, 16);

namespace nl
{
    JsonBase::JsonBase(JsonType type) :
        m_type(type)
    {
    }
}