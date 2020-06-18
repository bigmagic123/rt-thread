/*
 * Copyright (c) 2006-2020, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author         Notes
 * 2020-06-16     bigmagic       first version
 */

#include "drv_spi.h"
#include <rtthread.h>

rt_err_t rt_hw_spi_init(void)
{
    return RT_EOK;
}
INIT_DEVICE_EXPORT(rt_hw_spi_init);
