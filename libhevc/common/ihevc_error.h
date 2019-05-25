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
*  ihevc_error.h
*
* @brief
*  Definitions related to error handling for common modules
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

#ifndef _IHEVC_ERROR_H_
#define _IHEVC_ERROR_H_

/**
 * Enumerations for error codes used in the codec.
 * Not all these are expected to be returned to the application.
 * Only select few will be exported
 */
typedef enum
{
    /**
     *  No error
     */
    IHEVC_SUCCESS = 0,
    /**
     *  Start error code for decoder
     */
    IHEVC_DEC_ERROR_START = 0x100,

    /**
     *  Start error code for encoder
     */
    IHEVC_ENC_ERROR_START = 0x200,
    /**
     * Generic failure
     */
    IHEVC_FAIL                             = 0x7FFFFFFF
}IHEVC_ERROR_T;
#endif /* _IHEVC_ERROR_H_ */
