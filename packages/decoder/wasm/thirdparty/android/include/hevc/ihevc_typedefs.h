/******************************************************************************
*
* Copyright (C) 2012 Ittiam Systems Pvt Ltd, Bangalore
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at:
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*
******************************************************************************/
/**
*******************************************************************************
* @file
*  typedefs.h
*
* @brief
*  Type definitions used in the code
*
* @author
*  Srinivas T
*
* @remarks
*  None
*
*******************************************************************************
*/

#ifndef _IHEVC_TYPEDEFS_H_
#define _IHEVC_TYPEDEFS_H_


typedef unsigned char   UWORD8;
typedef unsigned short  UWORD16;
typedef unsigned int    UWORD32;

typedef signed char     WORD8;
typedef signed short    WORD16;
typedef signed int      WORD32;

typedef char            CHAR;

typedef double          DOUBLE;

typedef char           STRWORD8;



#ifndef MSVC

typedef unsigned long long ULWORD64;
typedef signed long long    LWORD64;

#else
typedef unsigned __int64    ULWORD64;
typedef __int64             LWORD64;


#endif
#endif /*   _IHEVC_TYPEDEFS_H_ */
