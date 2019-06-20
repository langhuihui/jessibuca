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
*  ihevcd_job_queue.h
*
* @brief
*  Contains functions for job queue
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

#ifndef _IHEVCD_JOB_QUEUE_H_
#define _IHEVCD_JOB_QUEUE_H_

typedef struct
{
    /** Pointer to buffer base which contains the jobs */
    void *pv_buf_base;

    /** Pointer to current address where new job can be added */
    void *pv_buf_wr;

    /** Pointer to current address from where next job can be obtained */
    void *pv_buf_rd;

    /** Pointer to end of job buffer */
    void *pv_buf_end;

    /** Mutex used to keep the functions thread-safe */
    void *pv_mutex;

    /** Flag to indicate jobq has to be terminated */
    WORD32 i4_terminate;
}jobq_t;

WORD32 ihevcd_jobq_ctxt_size(void);
void* ihevcd_jobq_init(void *pv_buf, WORD32 buf_size);
IHEVCD_ERROR_T ihevcd_jobq_free(jobq_t *ps_jobq);
IHEVCD_ERROR_T ihevcd_jobq_reset(jobq_t *ps_jobq);
IHEVCD_ERROR_T ihevcd_jobq_deinit(jobq_t *ps_jobq);
IHEVCD_ERROR_T ihevcd_jobq_terminate(jobq_t *ps_jobq);
IHEVCD_ERROR_T ihevcd_jobq_queue(jobq_t *ps_jobq, void *pv_job, WORD32 job_size, WORD32 blocking);
IHEVCD_ERROR_T ihevcd_jobq_dequeue(jobq_t *ps_jobq, void *pv_job, WORD32 job_size, WORD32 blocking);

#endif /* _IHEVCD_PROCESS_SLICE_H_ */
