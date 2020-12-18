/*
 * Copyright (c) 2006-2020, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author         Notes
 * 2020-12-06     bigmagic       first version
 */

#include <rtthread.h>
#include <rtdevice.h>
#include "drv_timer.h"

void usleep_delay(int delay)
{
    unsigned int compare = SYSTEM_TIMER_CLO + delay;
    while (SYSTEM_TIMER_CLO < compare);
}
