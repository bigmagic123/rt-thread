/*
 * Copyright (c) 2006-2020, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author         Notes
 * 2020-12-06     bigmagic       first version
 */

#ifndef __DRV_CLOCK_H__
#define __DRV_CLOCK_H__

#include <rthw.h>

#define CLK_BASE   (0xFE000000 + 0x101000)
#define CLK_LEN   0xA8
#define GPIO_LEN  0xB4

#define CLK_PASSWD  (0x5A<<24)

#define CLK_CTL_MASH(x)((x)<<9)
#define CLK_CTL_BUSY    (1 <<7)
#define CLK_CTL_KILL    (1 <<5)
#define CLK_CTL_ENAB    (1 <<4)
#define CLK_CTL_SRC(x) ((x)<<0)

#define CLK_SRCS 4

#define CLK_CTL_SRC_OSC  1  /* 19.2 MHz */
#define CLK_CTL_SRC_PLLC 5  /* 1000 MHz */
#define CLK_CTL_SRC_PLLD 6  /*  500 MHz for RPi 3, 750 MHz for RPi 4*/
#define CLK_CTL_SRC_HDMI 7  /*  216 MHz */

#define CLK_DIV_DIVI(x) ((x)<<12)
#define CLK_DIV_DIVF(x) ((x)<< 0)

#define CLK_GP0_CTL     __REG32(CLK_BASE + 0x70)
#define CLK_GP0_DIV     __REG32(CLK_BASE + 0x74)
#define CLK_GP1_CTL     __REG32(CLK_BASE + 0x78)
#define CLK_GP1_DIV     __REG32(CLK_BASE + 0x7C)
#define CLK_GP2_CTL     __REG32(CLK_BASE + 0x80)
#define CLK_GP2_DIV     __REG32(CLK_BASE + 0x84)
#define CLK_PCM_CTL     __REG32(CLK_BASE + 0x98)
#define CLK_PCM_DIV     __REG32(CLK_BASE + 0x9C)
#define CLK_PWM_CTL     __REG32(CLK_BASE + 0xA0)
#define CLK_PWM_DIV     __REG32(CLK_BASE + 0xA4)
#define CLK_UART_CTL    __REG32(CLK_BASE + 0xF0)
#define CLK_UART_DIV    __REG32(CLK_BASE + 0xF4)


#endif
