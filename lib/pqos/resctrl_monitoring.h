/*
 * BSD LICENSE
 *
 * Copyright(c) 2018 Intel Corporation. All rights reserved.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in
 *     the documentation and/or other materials provided with the
 *     distribution.
 *   * Neither the name of Intel Corporation nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.O
 *
 */

/**
 * @brief Internal header file for resctrl minitoring functions
 */


#ifndef __PQOS_RESCTRL_MON_H__
#define __PQOS_RESCTRL_MON_H__

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initializes resctrl structures used for OS monitoring interface
 *
 * @param cpu cpu topology structure
 * @param cap capabilities structure
 *
 * @return Operational status
 * @retval PQOS_RETVAL_OK success
 */
int resctrl_mon_init(const struct pqos_cpuinfo *cpu,
                     const struct pqos_cap *cap);

/**
 * @brief Shuts down monitoring sub-module for resctrl monitoring
 *
 * @return Operation status
 * @retval PQOS_RETVAL_OK success
 */
int resctrl_mon_fini(void);

/**
 * @brief This function starts resctrl event counters
 *
 * @param group monitoring structure
 *
 * @return Operation status
 * @retval PQOS_RETVAL_OK on success
 */
int resctrl_mon_start(struct pqos_mon_data *group);

/**
 * @brief Function to stop resctrl event counters
 *
 * @param group monitoring structure
 *
 * @return Operation status
 * @retval PQOS_RETVAL_OK on success
 */
int resctrl_mon_stop(struct pqos_mon_data *group);

/**
 * @brief This function polls all resctrl counters
 *
 * Reads counters for all events and stores values
 *
 * @param group monitoring structure
 *
 * @return Operation status
 * @retval PQOS_RETVAL_OK on success
 * @retval PQOS_RETVAL_ERROR if error occurs
 */
int resctrl_mon_poll(struct pqos_mon_data *group,
                     const enum pqos_mon_event event);

/**
 * @brief Check if event is supported by resctrl
 *
 * @param event PQoS event to check
 *
 * @retval 0 if not supported
 */
int resctrl_mon_is_event_supported(const enum pqos_mon_event event);

#ifdef __cplusplus
}
#endif

#endif /* __PQOS_RESCTRL_H__ */
