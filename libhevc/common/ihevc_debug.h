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
*  ihevc_debug.h
*
* @brief
*  Definitions for codec debugging
*
* @author
*  Ittiam
*
* @par List of Functions:
*
* @remarks
*  None
*
*******************************************************************************
*/
#ifndef _IHEVC_DEBUG_H_
#define _IHEVC_DEBUG_H_

#ifdef DEBUG_PRINT

#define DEBUG(...)                                                 \
{                                                                           \
    printf("\n[HEVC DBG] %s/%d:: ", __FUNCTION__, __LINE__);                \
    printf(__VA_ARGS__);                                                    \
}

#else //DEBUG_CODEC

#define DEBUG(...) {}

#endif //DEBUG_CODEC

#ifndef ASSERT_EXIT

#define ASSERT(x) assert((x))
//#define ASSERT(x) ihevcd_debug_ASSERT((x))

#else
#define ASSERT(x)                        \
{                                        \
    if (!(x))                            \
    {                                    \
        printf("ASSERT %s %d\n", __FILE__, __LINE__);              \
        exit(-1);                        \
    }                                    \
}
#endif

#endif /* _IHEVC_DEBUG_H_ */

