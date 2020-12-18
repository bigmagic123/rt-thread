/*
 * Copyright (c) 2006-2020, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author         Notes
 * 2020-12-06     bigmagic       first version
 */

#ifndef __DRV_PWM_H__
#define __DRV_PWM_H__

#include <rthw.h>

//rpi_DATA_2711_1p0.pdf
//PWM0 DMA is mapped to DMA channel5
//PWM1 DMA is mapped to DMA channel1
/*
GPIO    ALT0    ALT1    ALT2    ALT3    ALT4    ALT5
GPIO12  PWM0_0
GPIO13  PWM0_1
GPIO18                                          PWM0_0
GPIO19                                          PWM0_1
GPIO40  PWM1_0
GPIO41  PWM1_1
GPIO45  PWM0_1
*/

#define PWM_CTL(BASE)       __REG32(BASE + 0x0000)      /* PWM Control */
#define PWM_STA(BASE)       __REG32(BASE + 0x0004)      /* PWM Status */
#define PWM_DMAC(BASE)      __REG32(BASE + 0x0008)      /* PWM DMA Configuration */
#define PWM_RNG1(BASE)      __REG32(BASE + 0x0010)      /* PWM Channel 1 Range */
#define PWM_DAT1(BASE)      __REG32(BASE + 0x0014)      /* PWM Channel 1 Data */
#define PWM_FIF1(BASE)      __REG32(BASE + 0x0018)      /* PWM FIFO Input */
#define PWM_RNG2(BASE)      __REG32(BASE + 0x0020)      /* PWM Channel 2 Range */
#define PWM_DAT2(BASE)      __REG32(BASE + 0x0024)      /* PWM Channel 2 Data */

#define PWM_CTL_MSEN2      (1 << 15)
#define PWM_CTL_USEF2      (1 << 13)
#define PWM_CTL_POLA2      (1 << 12)
#define PWM_CTL_SBIT2      (1 << 11)
#define PWM_CTL_RPTL2      (1 << 10)
#define PWM_CTL_MODE2      (1 << 9)
#define PWM_CTL_PWEN2      (1 << 8)
#define PWM_CTL_MSEN1      (1 << 7)
#define PWM_CTL_CLRF       (1 << 6)
#define PWM_CTL_USEF1      (1 << 5)
#define PWM_CTL_POLA1      (1 << 4)
#define PWM_CTL_SBIT1      (1 << 3)
#define PWM_CTL_RPTL1      (1 << 2)
#define PWM_CTL_MODE       (1 << 1) //1:serialiser 0:pwm
#define PWM_CTL_PWEN1      (1 << 0)

#define PWM_STA_STA2       (1 << 10)
#define PWM_STA_STA1       (1 << 9)
#define PWM_STA_BERR       (1 << 8)
#define PWM_STA_GAPO2      (1 << 5)
#define PWM_STA_GAPO1      (1 << 4)
#define PWM_STA_RERR1      (1 << 3)
#define PWM_STA_WERR1      (1 << 2)
#define PWM_STA_EMPT1      (1 << 1)
#define PWM_STA_FULL1      (1 << 0)

#endif
