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
*  ihevcd_error.h
*
* @brief
*  Definitions related to error handling
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

#ifndef _IHEVCD_ERROR_H_
#define _IHEVCD_ERROR_H_

/**
 * Enumerations for error codes used in the codec.
 * Not all these are expected to be returned to the application.
 * Only select few will be exported
 */
typedef enum
{
    /**
     * VPS id more than MAX_VPS_CNT
     */
    IHEVCD_UNSUPPORTED_VPS_ID  = IVD_DUMMY_ELEMENT_FOR_CODEC_EXTENSIONS + 0x300,
    /**
     * SPS id more than MAX_SPS_CNT
     */

    IHEVCD_UNSUPPORTED_SPS_ID,
    /**
     * PPS id more than MAX_PPS_CNT
     */

    IHEVCD_UNSUPPORTED_PPS_ID,

    /**
     * Invelid Parameter while decoding
     */
    IHEVCD_INVALID_PARAMETER,

    /**
     * Invalid header
     */
    IHEVCD_INVALID_HEADER,

    /**
     * In sufficient memory allocated for MV Bank
     */
    IHEVCD_INSUFFICIENT_MEM_MVBANK,

    /**
     * In sufficient memory allocated for MV Bank
     */
    IHEVCD_INSUFFICIENT_MEM_PICBUF,

    /**
     * Buffer manager error
     */
    IHEVCD_BUF_MGR_ERROR,

    /**
     * No free MV Bank buffer available to store current pic
     */
    IHEVCD_NO_FREE_MVBANK,

    /**
     * No free picture buffer available to store current pic
     */
    IHEVCD_NO_FREE_PICBUF,
    /**
     * Reached slice header in header mode
     */
    IHEVCD_SLICE_IN_HEADER_MODE,

    /**
     * Ignore current slice and continue
     */
    IHEVCD_IGNORE_SLICE,

    /**
     * Reference Picture not found
     */
    IHEVCD_REF_PIC_NOT_FOUND,

    /**
     * Bit depth is greater than 8
     */
    IHEVCD_UNSUPPORTED_BIT_DEPTH,

    /**
     * Limit on the number of frames decoded
     */
    IHEVCD_NUM_FRAMES_LIMIT_REACHED,

    /**
     * VUI parameters not found
     */
    IHEVCD_VUI_PARAMS_NOT_FOUND,

    /**
     * SEI mastering parameters not found
     */
    IHEVCD_SEI_MASTERING_PARAMS_NOT_FOUND,

}IHEVCD_ERROR_T;
#endif /* _IHEVCD_ERROR_H_ */
