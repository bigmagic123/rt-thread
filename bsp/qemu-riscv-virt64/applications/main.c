/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-05-20     bigmagic     first version
 */

#include <rtthread.h>
#include <rthw.h>
#include <stdio.h>
#include <string.h>
#include <msh.h>

int main(void)
{
    rt_kprintf("Hello RISC-V!\n");
    char* dns_info = "dns e0 0 114.114.114.114";
    msh_exec(dns_info,rt_strlen(dns_info));
    return 0;
}
