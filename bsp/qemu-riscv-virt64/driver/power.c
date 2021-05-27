/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-05-27     bigmagic     first version
 */
#include <msh.h>
#include <rtthread.h>

#include "sbi.h"
#include "power.h"

void rt_hw_cpu_reset(void)
{
    *(uint32_t*)VIRT_POWER_ADDR = FINISHER_RESET;
}
MSH_CMD_EXPORT_ALIAS(rt_hw_cpu_reset, reboot, reset machine);

void rt_hw_cpu_poweroff(void)
{
    *(uint32_t*)VIRT_POWER_ADDR = FINISHER_PASS;
}
MSH_CMD_EXPORT_ALIAS(rt_hw_cpu_poweroff, poweroff, machine poweroff);
