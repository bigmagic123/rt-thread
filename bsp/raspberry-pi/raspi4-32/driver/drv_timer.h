/*
 * Copyright (c) 2006-2020, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author         Notes
 * 2020-12-06     bigmagic       first version
 */

#ifndef __DRV_TIMER_H__
#define __DRV_TIMER_H__

#include <rthw.h>

#define SYSTEM_TIMER_CS     HWREG32(SYSTEM_TIMER_BASE + 0x00)
#define SYSTEM_TIMER_CLO    HWREG32(SYSTEM_TIMER_BASE + 0x04)
#define SYSTEM_TIMER_CHI    HWREG32(SYSTEM_TIMER_BASE + 0x08)
#define SYSTEM_TIMER_C0     HWREG32(SYSTEM_TIMER_BASE + 0x0C)
#define SYSTEM_TIMER_C1     HWREG32(SYSTEM_TIMER_BASE + 0x10)
#define SYSTEM_TIMER_C2     HWREG32(SYSTEM_TIMER_BASE + 0x14)
#define SYSTEM_TIMER_C3     HWREG32(SYSTEM_TIMER_BASE + 0x18)

#define SYSTEM_TIMER_BASE   (0xFE000000 + 0x3000)

void usleep_delay(int delay);

#endif
