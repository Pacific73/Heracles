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
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <malloc.h>
#include <time.h>
#include <errno.h>

#include "pqos.h"
#include "log.h"
#include "types.h"
#include "resctrl.h"
#include "resctrl_monitoring.h"
#include "resctrl_alloc.h"
#include "os_allocation.h"


#define RESCTRL_PATH_INFO_L3_MON RESCTRL_PATH_INFO"/L3_MON"

/**
 * ---------------------------------------
 * Local data structures
 * ---------------------------------------
 */
static const struct pqos_cap *m_cap = NULL;
static const struct pqos_cpuinfo *m_cpu = NULL;

static int supported_events = 0;

/**
 * @brief Update monitoring capability structure with supported events
 *
 * @param cap pqos capability structure
 *
 * @return Operational Status
 * @retval PQOS_RETVAL_OK on success
 */
static void
set_mon_caps(const struct pqos_cap *cap)
{
        int ret;
        unsigned i;
        const struct pqos_capability *p_cap = NULL;

        ASSERT(cap != NULL);

        /* find monitoring capability */
        ret = pqos_cap_get_type(cap, PQOS_CAP_TYPE_MON, &p_cap);
        if (ret != PQOS_RETVAL_OK)
                return;

        /* update capabilities structure */
        for (i = 0; i < p_cap->u.mon->num_events; i++) {
                struct pqos_monitor *mon = &p_cap->u.mon->events[i];

                if (supported_events & mon->type)
                        mon->os_support = PQOS_OS_MON_RESCTRL;
        }
}

int
resctrl_mon_init(const struct pqos_cpuinfo *cpu, const struct pqos_cap *cap)
{
        int ret = PQOS_RETVAL_OK;
        char buf[64];
        FILE *fd;
        struct stat st;

        ASSERT(cpu != NULL);
        ASSERT(cap != NULL);

        /**
         * Check if resctrl is mounted
         */
        if (stat(RESCTRL_PATH_INFO, &st) != 0) {
                ret = resctrl_mount(PQOS_REQUIRE_CDP_OFF, PQOS_REQUIRE_CDP_OFF);
                if (ret != PQOS_RETVAL_OK) {
                        LOG_INFO("Unable to mount resctrl\n");
                        return PQOS_RETVAL_RESOURCE;
                }
        }

        /**
         * Resctrl monitiring not supported
         */
        if (stat(RESCTRL_PATH_INFO_L3_MON, &st) != 0)
                return PQOS_RETVAL_OK;

        /**
         * Discover supported events
         */
        fd = fopen(RESCTRL_PATH_INFO_L3_MON"/mon_features", "r");
        if (fd == NULL) {
                LOG_ERROR("Failed to obtain resctrl monitoring features\n");
                return PQOS_RETVAL_ERROR;
        }

        while (fgets(buf, sizeof(buf), fd) != NULL) {
                if (strncmp(buf, "llc_occupancy\n", sizeof(buf)) == 0) {
                        LOG_INFO("Detected resctrl support for "
                                 "LLC Occupancy\n");
                        supported_events |= PQOS_MON_EVENT_L3_OCCUP;
                        continue;
                }

                if (strncmp(buf, "mbm_local_bytes\n", sizeof(buf)) == 0) {
                        LOG_INFO("Detected resctrl support for "
                                 "Local Memory B/W\n");
                        supported_events |= PQOS_MON_EVENT_LMEM_BW;
                        continue;
                }

                if (strncmp(buf, "mbm_total_bytes\n", sizeof(buf)) == 0) {
                        LOG_INFO("Detected resctrl support "
                                "Total Memory B/W\n");
                        supported_events |= PQOS_MON_EVENT_TMEM_BW;
                }
        }

        if ((supported_events & PQOS_MON_EVENT_LMEM_BW) &&
                (supported_events & PQOS_MON_EVENT_TMEM_BW))
                supported_events |= PQOS_MON_EVENT_RMEM_BW;

        fclose(fd);

        /**
         * Update mon capabilities
         */
        set_mon_caps(cap);

        m_cap = cap;
        m_cpu = cpu;

        return ret;
}

int
resctrl_mon_fini(void)
{
        m_cap = NULL;
        m_cpu = NULL;

        return PQOS_RETVAL_OK;
}

/**
 * @brief Get core association
 *
 * @param [in] lcore core id
 * @param [out] class_id COS id
 *
 * @return Operational status
 * @retval PQOS_RETVAL_OK on success
 */
static int
resctrl_mon_assoc_get(const unsigned lcore, unsigned *class_id)
{
        int ret;
        unsigned max_cos;

        ret = resctrl_alloc_get_grps_num(m_cap, &max_cos);
        if (ret != PQOS_RETVAL_OK)
                return ret;

        if (max_cos == 0) {
                *class_id = 0;
                return PQOS_RETVAL_OK;
        }

        ret = os_alloc_assoc_get(lcore, class_id);
        if (ret != PQOS_RETVAL_OK)
                LOG_ERROR("Failed to retrieve core %u assotiation\n", lcore);

        return ret;
}

/**
 * @brief Get task association
 *
 * @param [in] tid task id
 * @param [out] class_id COS id
 *
 * @return Operational status
 * @retval PQOS_RETVAL_OK on success
 */
static int
resctrl_mon_assoc_get_pid(const pid_t tid, unsigned *class_id)
{
        int ret;
        unsigned max_cos;

        ret = resctrl_alloc_get_grps_num(m_cap, &max_cos);
        if (ret != PQOS_RETVAL_OK)
                return ret;

        if (max_cos == 0) {
                *class_id = 0;
                return PQOS_RETVAL_OK;
        }

        ret = os_alloc_assoc_get_pid(tid, class_id);
        if (ret != PQOS_RETVAL_OK)
                LOG_ERROR("Failed to retrieve task %d assotiation\n", tid);

        return ret;
}

/**
 * @brief Obtain path to monitoring group
 *
 * @param [in] class_id COS id
 * @param [in] resctrl_group mon group name
 * @param [in] file file name insige group
 * @param [out] buf Buffer to store path
 * @param [in] buf_size buffer size
 */
static void
resctrl_mon_group_path(const unsigned class_id,
                       const char *resctrl_group,
                       const char *file,
                       char *buf,
                       const unsigned buf_size)
{
        if (resctrl_group == NULL) {
                if (class_id == 0)
                        snprintf(buf, buf_size, RESCTRL_PATH);
                else
                        snprintf(buf, buf_size, RESCTRL_PATH"/COS%u",
                                 class_id);
        } else if (class_id == 0)
                snprintf(buf, buf_size, RESCTRL_PATH"/mon_groups/%s",
                         resctrl_group);
        else
                snprintf(buf, buf_size, RESCTRL_PATH"/COS%u/mon_groups/%s",
                         class_id, resctrl_group);

        if (file != NULL)
                strncat(buf, file, buf_size - strlen(buf));
}

/**
 * @brief Write CPU mask to file
 *
 * @param [in] class_id COS id
 * @param [in] resctrl_group mon group name
 * @param [in] mask CPU mask to write
 *
 * @return Operational status
 * @retval PQOS_RETVAL_OK on success
 */
static int
resctrl_mon_cpumask_write(const unsigned class_id,
                          const char *resctrl_group,
                          const struct resctrl_cpumask *mask)
{
        FILE *fd;
        char path[128];
        int ret;

        resctrl_mon_group_path(class_id, resctrl_group, "/cpus", path,
                               sizeof(path));

        fd = fopen(path, "w");
        if (fd == NULL)
                return PQOS_RETVAL_ERROR;

        ret = resctrl_cpumask_write(fd, mask);

        if (fclose(fd) != 0)
                return PQOS_RETVAL_ERROR;

        return ret;
}

/**
 * @brief Read CPU mask from file
 *
 * @param [in] class_id COS id
 * @param [in] resctrl_group mon group name
 * @param [out] mask CPU mask to write
 *
 * @return Operational status
 * @retval PQOS_RETVAL_OK on success
 */
static int
resctrl_mon_cpumask_read(const unsigned class_id,
                         const char *resctrl_group,
                         struct resctrl_cpumask *mask)
{
        FILE *fd;
        char path[128];
        int ret;

        resctrl_mon_group_path(class_id, resctrl_group, "/cpus", path,
                               sizeof(path));

        fd = fopen(path, "r");
        if (fd == NULL)
                return PQOS_RETVAL_ERROR;

        ret = resctrl_cpumask_read(fd, mask);

        fclose(fd);

        return ret;
}

/**
 * @brief Assign core to monitoring group
 *
 * @param[in] name Monitoring group name
 * @param[in] lcore core id
 *
 * @return Operation status
 * @retval PQOS_RETVAL_OK on success
 * @retval PQOS_RETVAL_ERROR if error occurs
 */
static int
resctrl_mon_core_assign(const char *name, const unsigned lcore)
{
        unsigned class_id = 0;
        int ret;
        char path[128];
        struct stat st;
        struct resctrl_cpumask cpumask;

        ret = resctrl_mon_assoc_get(lcore, &class_id);
        if (ret != PQOS_RETVAL_OK)
                return ret;

        resctrl_mon_group_path(class_id, name, NULL, path, sizeof(path));
        if (stat(path, &st) != 0 && mkdir(path, 0755) == -1) {
                LOG_DEBUG("Failed to create resctrl monitoring group %s!\n",
                          path);
                return PQOS_RETVAL_BUSY;
        }

        ret = resctrl_mon_cpumask_read(class_id, name, &cpumask);
        if (ret != PQOS_RETVAL_OK)
                return ret;

        resctrl_cpumask_set(lcore, &cpumask);

        ret = resctrl_mon_cpumask_write(class_id, name, &cpumask);
        if (ret != PQOS_RETVAL_OK)
                LOG_ERROR("Could not assign core %u to resctrl monitoring "
                          "group\n", lcore);

        return ret;
}

/**
 * @brief Assign pid to monitoring group
 *
 * @param[in] name Monitoring group name
 * @param[in] tid task id
 *
 * @return Operation status
 * @retval PQOS_RETVAL_OK on success
 * @retval PQOS_RETVAL_ERROR if error occurs
 */
static int
resctrl_mon_task_assign(const char *name, const pid_t tid)
{
        unsigned class_id;
        int ret;
        char path[128];
        FILE *fd;
        struct stat st;

        ret = resctrl_mon_assoc_get_pid(tid, &class_id);
        if (ret != PQOS_RETVAL_OK)
                return ret;

        resctrl_mon_group_path(class_id, name, NULL, path, sizeof(path));
        if (stat(path, &st) != 0 && mkdir(path, 0755) == -1) {
                LOG_DEBUG("Failed to create resctrl monitoring group %s!\n",
                          path);
                return PQOS_RETVAL_BUSY;
        }

        strncat(path, "/tasks", sizeof(path) - strlen(path));
        fd = fopen(path, "w");
        if (fd == NULL)
                return PQOS_RETVAL_ERROR;

        fprintf(fd, "%d\n", tid);

        if (fclose(fd) != 0) {
                LOG_ERROR("Could not assign TID %d to resctrl monitoring "
                          "group\n", tid);

                return PQOS_RETVAL_ERROR;
        }

        return PQOS_RETVAL_OK;
}

int
resctrl_mon_start(struct pqos_mon_data *group)
{
        char *resctrl_group = NULL;
        char buf[128];
        int ret;
        unsigned i;

        ASSERT(group != NULL);
        ASSERT(group->tid_nr == 0 || group->tid_map != NULL);
        ASSERT(group->num_cores == 0 || group->cores != NULL);

        ret = resctrl_lock_exclusive();
        if (ret != PQOS_RETVAL_OK)
                goto resctrl_mon_start_exit;

        /**
         * Create new monitoring gorup
         */
        if (group->resctrl_group == NULL) {
                snprintf(buf, sizeof(buf), "%s-%p-%ld",
                        group->num_pids > 0 ? "pid" : "core", group,
                        (long int) time(NULL));

                resctrl_group = strdup(buf);
                if (resctrl_group == NULL) {
                        LOG_DEBUG("Memory allocation failed\n");
                        ret = PQOS_RETVAL_ERROR;
                        goto resctrl_mon_start_exit;
                }
        /**
         * Reuse group
         */
        } else
                resctrl_group = group->resctrl_group;

        /**
         * Add pids to the resctrl group
         */
        for (i = 0; i < group->tid_nr; i++) {
                ret = resctrl_mon_task_assign(resctrl_group, group->tid_map[i]);
                if (ret != PQOS_RETVAL_OK)
                        goto resctrl_mon_start_exit;
        }

        /**
         * Add cores to the resctrl group
         */
        for (i = 0; i < group->num_cores; i++) {
                ret = resctrl_mon_core_assign(resctrl_group, group->cores[i]);
                if (ret != PQOS_RETVAL_OK)
                        goto resctrl_mon_start_exit;
        }

        group->resctrl_group = resctrl_group;

 resctrl_mon_start_exit:
        resctrl_lock_release();

        if (ret != PQOS_RETVAL_OK && group->resctrl_group != resctrl_group)
                free(resctrl_group);

        return ret;
}

/**
 * @brief Function to stop resctrl event counters
 *
 * @param group monitoring structure
 *
 * @return Operation status
 * @retval PQOS_RETVAL_OK on success
 */
int
resctrl_mon_stop(struct pqos_mon_data *group)
{
        int ret;
        unsigned max_cos;
        unsigned cos;
        unsigned i;

        ASSERT(group != NULL);

        ret = resctrl_alloc_get_grps_num(m_cap, &max_cos);
        if (ret != PQOS_RETVAL_OK)
                return ret;

        ret = resctrl_lock_exclusive();
        if (ret != PQOS_RETVAL_OK)
                goto resctrl_mon_stop_exit;

        if (group->resctrl_group != NULL) {
                cos = 0;
                do {
                        char buf[128];
                        struct stat st;

                        resctrl_mon_group_path(cos, group->resctrl_group, NULL,
                                               buf, sizeof(buf));
                        if (stat(buf, &st) == 0 && rmdir(buf) != 0) {
                                ret = PQOS_RETVAL_ERROR;
                                goto resctrl_mon_stop_exit;
                        }

                        cos++;
                } while (cos < max_cos);

                free(group->resctrl_group);
                group->resctrl_group = NULL;

        } else if (group->num_pids > 0) {
                /**
                 * Add pids to the default group
                 */
                for (i = 0; i < group->tid_nr; i++) {
                        ret = resctrl_mon_task_assign(NULL, group->tid_map[i]);
                        if (ret != PQOS_RETVAL_OK)
                                goto resctrl_mon_stop_exit;
                }

        } else
                return PQOS_RETVAL_PARAM;

 resctrl_mon_stop_exit:
        resctrl_lock_release();

        return ret;
}

/**
 * @brief Gives the difference between two values with regard to the possible
 *        overrun
 *
 * @param old_value previous value
 * @param new_value current value
 * @return difference between the two values
 */
static uint64_t
get_delta(const uint64_t old_value, const uint64_t new_value)
{
        if (old_value > new_value)
                return (UINT64_MAX - old_value) + new_value;
        else
                return new_value - old_value;
}

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
int
resctrl_mon_poll(struct pqos_mon_data *group, const enum pqos_mon_event event)
{
        int ret;
        const char *name;
        uint64_t value = 0;
        unsigned max_cos;
        unsigned *sockets = NULL;
        unsigned sockets_num;
        unsigned cos, sock;
        uint64_t old_value;

        ASSERT(group != NULL);
        ASSERT(m_cap != NULL);
        ASSERT(m_cpu != NULL);

        switch (event) {
        case PQOS_MON_EVENT_L3_OCCUP:
                name = "llc_occupancy";
                break;
        case PQOS_MON_EVENT_LMEM_BW:
                name = "mbm_local_bytes";
                break;
        case PQOS_MON_EVENT_TMEM_BW:
                name = "mbm_total_bytes";
                break;
        default:
                LOG_ERROR("Unknown resctrl event\n");
                return PQOS_RETVAL_PARAM;
                break;
        }

        ret = resctrl_alloc_get_grps_num(m_cap, &max_cos);
        if (ret != PQOS_RETVAL_OK)
                return ret;

        sockets = pqos_cpu_get_sockets(m_cpu, &sockets_num);
        if (sockets == NULL || sockets_num == 0) {
                ret = PQOS_RETVAL_ERROR;
                goto resctrl_mon_poll_exit;
        }

        /**
         * Read counters for each COS group and socket
         */
        cos = 0;
        do {
                char buf[128];
                struct stat st;

                resctrl_mon_group_path(cos, group->resctrl_group, NULL, buf,
                                       sizeof(buf));
                cos++;
                if (stat(buf, &st) != 0)
                        continue;

                for (sock = 0; sock < sockets_num; sock++) {
                        char path[128];
                        FILE *fd;
                        unsigned long long counter;

                        snprintf(path, sizeof(path),
                                 "%s/mon_data/mon_L3_%02u/%s", buf, sock, name);
                        fd = fopen(path, "r");
                        if (fd == NULL) {
                                ret = PQOS_RETVAL_ERROR;
                                goto resctrl_mon_poll_exit;
                        }
                        if (fscanf(fd, "%llu", &counter) == 1)
                                value += counter;
                        fclose(fd);
                }

        } while (cos < max_cos);

        /**
         * Set value
         */
        switch (event) {
        case PQOS_MON_EVENT_L3_OCCUP:
                group->values.llc = value;
                break;
        case PQOS_MON_EVENT_LMEM_BW:
                old_value = group->values.mbm_local;
                group->values.mbm_local = value;
                group->values.mbm_local_delta =
                        get_delta(old_value, group->values.mbm_local);
                break;
        case PQOS_MON_EVENT_TMEM_BW:
                old_value = group->values.mbm_total;
                group->values.mbm_total = value;
                group->values.mbm_total_delta =
                        get_delta(old_value, group->values.mbm_total);
                break;
        default:
                return PQOS_RETVAL_ERROR;
        }

 resctrl_mon_poll_exit:
        free(sockets);

        return ret;
}

int
resctrl_mon_is_event_supported(const enum pqos_mon_event event)
{
        return (supported_events & event) == event;
}
