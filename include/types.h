/****************************************************************************
 * Copyright (c) 2024 Hidegi
 *
 * This software is provided ‘as-is’, without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 * claim that you wrote the original software. If you use this software
 * in a product, an acknowledgment in the product documentation would be
 * appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and must not be
 * misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source
 * distribution.
 ****************************************************************************/
#ifndef BMT_TYPES_H
#define BMT_TYPES_H
typedef float BMT_float;
typedef double BMT_double;

typedef char BMT_char;
typedef unsigned char BMT_uchar;
typedef signed char BMT_int8;
typedef unsigned char BMT_uint8;
typedef BMT_int8 BMT_byte;
typedef BMT_uint8 BMT_ubyte;

typedef signed short BMT_int16;
typedef unsigned short BMT_uint16;
typedef BMT_int16 BMT_short;
typedef BMT_uint16 BMT_ushort;

#define BMT_FALSE 0
#define BMT_TRUE 1
typedef signed int BMT_int32;
typedef unsigned int BMT_uint32;
typedef BMT_int32 BMT_int;
typedef BMT_uint32 BMT_uint;
typedef BMT_int BMT_bool;

#if defined(BMT_MSC_VER)
typedef signed __int64 BMT_int64;
typedef unsigned __int64 BMT_uint64;
#else
typedef signed long long BMT_int64;
typedef unsigned long long BMT_uint64;
#endif // defined
typedef BMT_int64 BMT_long;
typedef BMT_uint64 BMT_ulong;

typedef BMT_ulong BMT_size;
typedef BMT_size BMT_index;
typedef BMT_ulong BMT_hash;
#endif