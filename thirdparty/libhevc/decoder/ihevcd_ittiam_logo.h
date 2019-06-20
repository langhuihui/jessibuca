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
/*****************************************************************************/
/*                                                                           */
/*  File Name         : ihevcd_ittiam_logo.h.h                               */
/*                                                                           */
/*  Description       : This file contains all the necessary function headers*/
/*                      to insert ittiam logo to a yuv buffer.               */
/*                                                                           */
/*  List of Functions : None                                                 */
/*                                                                           */
/*  Issues / Problems : None                                                 */
/*                                                                           */
/*  Revision History  :                                                      */
/*                                                                           */
/*         DD MM YYYY   Author(s)       Changes (Describe the changes made)  */
/*         10 10 2005   Ittiam          Draft                                */
/*                                                                           */
/*****************************************************************************/

#ifndef LOGO_INSERT_H
#define LOGO_INSERT_H

//#define LOGO_EN

#define LOGO_WD 90
#define LOGO_HT 36

#define LOGO_WD_Y       LOGO_WD
#define LOGO_HT_Y       LOGO_HT

#define LOGO_WD_RGBA8888       160
#define LOGO_HT_RGBA8888       64

#define LOGO_WD_RGB565       160
#define LOGO_HT_RGB565       64

#define LOGO_WD_444_UV  LOGO_WD
#define LOGO_HT_444_UV  LOGO_HT


#define LOGO_WD_420_UV  (LOGO_WD >> 1)
#define LOGO_HT_420_UV  (LOGO_HT >> 1)

#define LOGO_WD_420SP_UV  (LOGO_WD)
#define LOGO_HT_420SP_UV  (LOGO_HT >> 1)

#define LOGO_WD_420SP_VU  (LOGO_WD)
#define LOGO_HT_420SP_VU  (LOGO_HT >> 1)

#define LOGO_WD_422_UV  (LOGO_WD >> 1)
#define LOGO_HT_422_UV  (LOGO_HT)

#define LOGO_WD_422V_UV  (LOGO_WD)
#define LOGO_HT_422V_UV  (LOGO_HT >> 1)

#define LOGO_WD_411_UV  (LOGO_WD >> 2)
#define LOGO_HT_411_UV  (LOGO_HT)

#define LOGO_CODEC_WD 80
#define LOGO_CODEC_HT  24

#define LOGO_CODEC_WD_Y       LOGO_CODEC_WD
#define LOGO_CODEC_HT_Y       LOGO_CODEC_HT


#define LOGO_CODEC_WD_444_UV  LOGO_CODEC_WD
#define LOGO_CODEC_HT_444_UV  LOGO_CODEC_HT


#define LOGO_CODEC_WD_420_UV  (LOGO_CODEC_WD >> 1)
#define LOGO_CODEC_HT_420_UV  (LOGO_CODEC_HT >> 1)

#define LOGO_CODEC_WD_420SP_UV  (LOGO_CODEC_WD)
#define LOGO_CODEC_HT_420SP_UV  (LOGO_CODEC_HT >> 1)

#define LOGO_CODEC_WD_420SP_VU  (LOGO_CODEC_WD)
#define LOGO_CODEC_HT_420SP_VU  (LOGO_CODEC_HT >> 1)

#define LOGO_CODEC_WD_422_UV  (LOGO_CODEC_WD >> 1)
#define LOGO_CODEC_HT_422_UV  (LOGO_CODEC_HT)

#define LOGO_CODEC_WD_422V_UV  (LOGO_CODEC_WD)
#define LOGO_CODEC_HT_422V_UV  (LOGO_CODEC_HT >> 1)

#define LOGO_CODEC_WD_411_UV  (LOGO_CODEC_WD >> 2)
#define LOGO_CODEC_HT_411_UV  (LOGO_CODEC_HT)




#define START_X_ITT_LOGO        0
#define START_Y_ITT_LOGO        0

#define WD_ITT_LOGO             128
#define HT_ITT_LOGO             60

void ihevcd_insert_logo(UWORD8 *buf_y, UWORD8 *buf_u, UWORD8 *buf_v,
                        UWORD32 stride,
                        UWORD32 x_pos,
                        UWORD32 y_pos,
                        UWORD32 yuv_fmt,
                        UWORD32 u4_disp_wd,
                        UWORD32 u4_disp_ht);

#ifdef LOGO_EN
#define INSERT_LOGO(buf_y, buf_u, buf_v, stride, x_pos, y_pos, yuv_fmt,disp_wd,disp_ht) ihevcd_insert_logo(buf_y, buf_u, buf_v, stride, x_pos, y_pos, yuv_fmt,disp_wd,disp_ht);
#else
#define INSERT_LOGO(buf_y, buf_u, buf_v, stride, x_pos, y_pos, yuv_fmt,disp_wd,disp_ht)
#endif

#endif /* LOGO_INSERT_H */

