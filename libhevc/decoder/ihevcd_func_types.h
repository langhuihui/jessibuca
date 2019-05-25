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
*  ihevcd_func_types.h
*
* @brief
*  Defines different types of function implementations  Eg C, Cortex A8
* Intrinsics, Neon assembly etc
*
* @author
*  Harish
*
* @par List of Functions:
*
* @remarks
*  None
*
*******************************************************************************
*/
#ifndef _ihevcd_func_types_H_
#define _ihevcd_func_types_H_


/* C Model : No platform specific intrinsics or inline assemblies */
#define    C            0

/* Cortex Ax intrinsics */
#define    CXAINTR      10

/* Neon intrinsics */
#define    NEONINTR     11

/* X86 intrinsics */
#define    X86INTR      12

/* X64 intrinsics */
#define    X64INTR      13

/* Atom intrinsics */
#define    ATOMINTR       14

/* Cortex Ax assembly */
#define    CXAASM       20

/* Neon assembly */
#define    NEONASM      21

/* X86 assembly */
#define    X86ASM       22


#endif /* _ihevcd_func_types_H_ */
