/*
 * Copyright (c) 2006-2020, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author         Notes
 * 2020-11-08     bigmagic       first version
 */

#include <rtthread.h>
#include <rtdevice.h>

#include "drv_xpt2046.h"
//http://www.lcdwiki.com/MHS-3.5inch_RPi_Display

//XPT2049
#define     READ_X                (0xD0)
#define     READ_Y                (0x90)

#define     TFT_WIDTH             (320)
#define     TFT_HEIGHT            (480)
//freq
#define     TOUCH_SPI_MAX_FREQ    (10*1000)

#define     TP_IRQ_PIN            (17)
#define     TOUCH_DEVICE_NAME     ("spi0.1")

static struct rt_semaphore touch_ack;

static rt_thread_t touch_tid = RT_NULL;
#define TOUCH_THREAD_STACK_SIZE     (1024)
#define TOUCH_THREAD_PRIORITY       (20)
#define TOUCH_THREAD_TIMESLICE      (10)

static void touch_read_x_y(void *dev, rt_uint16_t *x, rt_uint16_t *y)
{
    struct rt_spi_device *touch_dev = (struct rt_spi_device *)dev;
    struct rt_spi_message msg1,msg2,msg3,msg4;
    rt_uint16_t readx_val,ready_val;
    rt_uint8_t readx[2];
    rt_uint8_t ready[2];

    int read_x_id = READ_X;
    int read_y_id = READ_Y;

    msg1.send_buf   = &read_x_id;
    msg1.recv_buf   = RT_NULL;
    msg1.length     = 1;
    msg1.cs_take    = 1;
    msg1.cs_release = 0;
    msg1.next       = &msg2;

    msg2.send_buf   = RT_NULL;
    msg2.recv_buf   = &readx[0];
    msg2.length     = 2;
    msg2.cs_take    = 0;
    msg2.cs_release = 0;
    msg2.next       = &msg3;

    msg3.send_buf   = &read_y_id;
    msg3.recv_buf   = RT_NULL;
    msg3.length     = 1;
    msg3.cs_take    = 0;
    msg3.cs_release = 0;
    msg3.next       = &msg4;

    msg4.send_buf   = RT_NULL;
    msg4.recv_buf   = &ready[0];
    msg4.length     = 2;
    msg4.cs_take    = 0;
    msg4.cs_release = 1;
    msg4.next       = RT_NULL;

    rt_spi_transfer_message(touch_dev, &msg1);

    readx_val = ((readx[0] << 8) | readx[1]) >> 4;
    ready_val = ((ready[0] << 8) | ready[1]) >> 4;

    *x = readx_val;
    *y = ready_val;
}

/*
XPT2046:Width:320 High:480
no pressed:(0x800,0xfff)
---ETH----USB-----------------------
| (0x800,0x800)      (0xfff,0x800) |
|                                  |
| (0x800,0xFFF)      (0xfff,0xfff) |
------------------------------------
*/
#define XMIN 0x800
#define YMAX 0xfff 
void read_tp(void *dev, rt_uint16_t *x, rt_uint16_t *y)
{
    struct rt_spi_device *touch_dev = (struct rt_spi_device *)dev;
    rt_uint8_t try = 0;
    uint16_t _y[5] = {0,0,0,0,0};
    uint16_t _x[5] = {0,0,0,0,0};
    uint16_t x_val = 0;
    uint16_t y_val = 0;
    uint16_t cur_x = 0;
    uint16_t cur_y = 0;
    int index = 0;

    while(1)
    {
        try = try + 1;
        touch_read_x_y(touch_dev, x, y);
        if((*x > XMIN) && (*y < YMAX))
        {
            _x[index] = *x;
            _y[index] = *y;
            index = index + 1;
        }
        if(index == 5)
        {
            break;
        }

        if(try > 10)
        {
            break;
        }
    }

    x_val = (_x[0] + _x[1] + _x[2] + _x[3]+ _x[4]) / index;
    y_val = (_y[0] + _y[1] + _y[2] + _y[3]+ _y[4]) / index;

    cur_x = (x_val - 0x800) * TFT_WIDTH / 0x800;
    cur_y = (y_val - 0x800) * TFT_HEIGHT / 0x800;

    if((cur_x < TFT_WIDTH) && (cur_y < TFT_HEIGHT))
    {
        *x = TFT_WIDTH - cur_x;
        *y = TFT_HEIGHT - cur_y;
    }
    else
    {
        *x = 0;
        *y = 0;
    }
}

static void touch_thread_entry(void *param)
{
    rt_uint16_t x,y;
    struct rt_spi_device *touch_dev;
    touch_dev = (struct rt_spi_device *)rt_device_find(TOUCH_DEVICE_NAME);
    touch_dev->config.max_hz = TOUCH_SPI_MAX_FREQ;
    if (!touch_dev)
    {
        rt_kprintf("no %s!\n", TOUCH_DEVICE_NAME);
    }

    while (1)
    {
        rt_sem_take(&touch_ack, RT_WAITING_FOREVER);
        read_tp(touch_dev, &x, &y);
        if((x!= 0) && (y !=0))
        {
            rt_kprintf("%d:%d\n", x,y);
        }
        rt_pin_mode(TP_IRQ_PIN, PIN_MODE_INPUT_PULLUP);
    }
}

static void touch_readly(void *args)
{
    if(rt_pin_read(TP_IRQ_PIN) == PIN_LOW)
    {
        rt_pin_mode(TP_IRQ_PIN, PIN_MODE_OUTPUT);
        rt_pin_write(TP_IRQ_PIN,PIN_HIGH);
        rt_sem_release(&touch_ack);
    }
}

static void hw_xpt2049_touch_init(void)
{
    //touch sem
    rt_sem_init(&touch_ack, "touch_ack", 0, RT_IPC_FLAG_FIFO);

    touch_tid = rt_thread_create("touch",
                            touch_thread_entry, RT_NULL,
                            TOUCH_THREAD_STACK_SIZE,
                            TOUCH_THREAD_PRIORITY, TOUCH_THREAD_TIMESLICE);
    if (touch_tid != RT_NULL)
        rt_thread_startup(touch_tid);

    rt_pin_mode(TP_IRQ_PIN, PIN_MODE_INPUT_PULLUP);
    rt_pin_attach_irq(TP_IRQ_PIN, PIN_IRQ_MODE_LOW_LEVEL, touch_readly, RT_NULL);
    rt_pin_irq_enable(TP_IRQ_PIN, PIN_IRQ_ENABLE);

}
INIT_DEVICE_EXPORT(hw_xpt2049_touch_init);
