/*
 * Copyright (c) 2006-2020, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author         Notes
 * 2020-11-08     bigmagic       first version
 */
//#include <pin.h>
#include <rtthread.h>
#include <rtdevice.h>

#include "drv_ili9486.h"

//http://www.lcdwiki.com/MHS-3.5inch_RPi_Display
#define LCD_DEVICE_NAME       ("spi0.0")
//waveshare
#define LCD_SCREEN_WIDTH      (480)
#define LCD_SCREEN_HEIGHT     (320)

#define LCD_RESET_PIN         (25)
#define LCD_RS_PIN            (24)

#define LCD_SPI_FREQ_MAX      (125*1000*1000)

uint16_t LCD_HEIGHT = LCD_SCREEN_HEIGHT;
uint16_t LCD_WIDTH  = LCD_SCREEN_WIDTH;

#define SCREEN_VERTICAL_1           (0)
#define SCREEN_HORIZONTAL_1         (1)
#define SCREEN_VERTICAL_2           (2)
#define SCREEN_HORIZONTAL_2         (3)

//rgb565 lcd buffer
uint16_t _lcd_buffer[LCD_SCREEN_WIDTH * LCD_SCREEN_HEIGHT];

static struct rt_spi_device *lcd_dev;

static inline void send_cmd(void)
{
    rt_pin_write(LCD_RS_PIN, PIN_LOW);
}

static inline void send_data(void)
{
    rt_pin_write(LCD_RS_PIN, PIN_HIGH);
}

void writeData16(rt_uint16_t data)
{
    rt_uint8_t send_data[2];
    send_data[1] = data & 0x00FF;
    send_data[0] = ((data >> 8) & 0x00FF);
    rt_spi_transfer(lcd_dev, &send_data[0], RT_NULL, 2);
}

void writeData(void* dev,rt_uint8_t data)
{
    struct rt_spi_device *lcd_dev = (struct rt_spi_device *)dev;
    writeData16((rt_uint16_t)(data));
}

void writeCommand(void* dev, rt_uint8_t cmd)
{
    struct rt_spi_device *lcd_dev = (struct rt_spi_device *)dev;
    send_cmd();
    writeData16((rt_uint16_t)(cmd));
    send_data();
}

void lcd_write_commmand(rt_uint8_t cmd)
{
    writeCommand(lcd_dev, cmd);
}

void lcd_write_data(rt_uint8_t data)
{
    writeData(lcd_dev, data);
}

/*Ser rotation of the screen - changes x0 and y0*/
static inline void lcd_set_rotation(uint8_t rotation) 
{
    writeCommand(lcd_dev, 0x36);
    rt_thread_mdelay(100);

    switch(rotation) {
        case SCREEN_VERTICAL_1:
            writeData(lcd_dev, 0x48);
            LCD_WIDTH  = 320;
            LCD_HEIGHT = 480;
            break;
        case SCREEN_HORIZONTAL_1:
            writeData(lcd_dev, 0x28);
            LCD_WIDTH  = 480;
            LCD_HEIGHT = 320;
            break;
        case SCREEN_VERTICAL_2:
            writeData(lcd_dev, 0x98);
            LCD_WIDTH  = 320;
            LCD_HEIGHT = 480;
            break;
        case SCREEN_HORIZONTAL_2:
            writeData(lcd_dev, 0xF8);
            LCD_WIDTH  = 480;
            LCD_HEIGHT = 320;
            break;
        default:
            //EXIT IF SCREEN ROTATION NOT VALID!
            break;
    }
}

static inline void lcd_show(void) 
{
    int i, j;
    lcd_write_commmand(0x2B);
    lcd_write_data(0x00);
    lcd_write_data(0x00);
    lcd_write_data(0x01);
    lcd_write_data(0x3F);

    lcd_write_commmand(0x2A);
    lcd_write_data(0x00);
    lcd_write_data(0x00);
    lcd_write_data(0x01);
    lcd_write_data(0xE0);

    lcd_write_commmand(0x2C); // Memory write?

    for (i = 0 ; i < 30  ; i ++) 
    {
        uint16_t *tx_data = (uint16_t*)&_lcd_buffer[5120*i];
        int32_t data_sz = 5120;
        for( j=0; j<data_sz; j++)  
        {
            writeData16(tx_data[j]);
        }
    }
}

void ili9486_flush(const void* buf, uint32_t size) 
{
    if(size < LCD_WIDTH * LCD_HEIGHT* 4)
        return;

    uint32_t *src = (uint32_t*)buf;
    uint32_t sz = LCD_HEIGHT*LCD_WIDTH;
    uint32_t i;

    for (i = 0; i < sz; i++) 
    {
        register uint32_t s = src[i];
        register uint8_t r = (s >> 16) & 0xff;
        register uint8_t g = (s >> 8)  & 0xff;
        register uint8_t b = s & 0xff;
        _lcd_buffer[i] = ((r >> 3) <<11) | ((g >> 3) << 6) | (b >> 3);
    }
    lcd_show();
}

static void lcd_init(void)
{
    writeCommand(lcd_dev, 0x28);
    rt_thread_mdelay(150);

    writeCommand(lcd_dev, 0x3A);    // Interface Pixel Format
    writeData(lcd_dev, 0x55);       // 16 bit/pixe

    writeCommand(lcd_dev, 0xC2);    // Interface Pixel Format
    writeData(lcd_dev, 0x44);

    writeCommand(lcd_dev, 0xC5);     // VCOM Control
    writeData(lcd_dev, 0x00);
    writeData(lcd_dev, 0x00);
    writeData(lcd_dev, 0x00);
    writeData(lcd_dev, 0x00);

    writeCommand(lcd_dev, 0xE0);     // PGAMCTRL(Positive Gamma Control)
    writeData(lcd_dev, 0x0F);
    writeData(lcd_dev, 0x1F);
    writeData(lcd_dev, 0x1C);
    writeData(lcd_dev, 0x0C);
    writeData(lcd_dev, 0x0F);
    writeData(lcd_dev, 0x08);
    writeData(lcd_dev, 0x48);
    writeData(lcd_dev, 0x98);
    writeData(lcd_dev, 0x37);
    writeData(lcd_dev, 0x0A);
    writeData(lcd_dev, 0x13);
    writeData(lcd_dev, 0x04);
    writeData(lcd_dev, 0x11);
    writeData(lcd_dev, 0x0D);
    writeData(lcd_dev, 0x00);

    writeCommand(lcd_dev, 0xE1);     // NGAMCTRL (Negative Gamma Correction)
    writeData(lcd_dev, 0x0F);
    writeData(lcd_dev, 0x32);
    writeData(lcd_dev, 0x2E);
    writeData(lcd_dev, 0x0B);
    writeData(lcd_dev, 0x0D);
    writeData(lcd_dev, 0x05);
    writeData(lcd_dev, 0x47);
    writeData(lcd_dev, 0x75);
    writeData(lcd_dev, 0x37);
    writeData(lcd_dev, 0x06);
    writeData(lcd_dev, 0x10);
    writeData(lcd_dev, 0x03);
    writeData(lcd_dev, 0x24);
    writeData(lcd_dev, 0x20);
    writeData(lcd_dev, 0x00);

    writeCommand(lcd_dev, 0x11); // Sleep out, also SW reset
    rt_thread_mdelay(150);

    writeCommand(lcd_dev, 0x20);   // Display Inversion OFF   RPi LCD (A)
    //writeCommand(lcd_dev, 0x21); // Display Inversion ON    RPi LCD (B)

    lcd_set_rotation(SCREEN_HORIZONTAL_1);
    writeCommand(lcd_dev, 0x29);   // Display ON
    rt_thread_mdelay(150);
}

static inline lcd_reset(void)
{
    //Reset signal, low reset (pin22)
    rt_pin_mode(LCD_RESET_PIN,PIN_MODE_OUTPUT);

    rt_pin_write(LCD_RESET_PIN, PIN_HIGH);
    rt_thread_mdelay(100);
    rt_pin_write(LCD_RESET_PIN, PIN_LOW);
    rt_thread_mdelay(100);
    rt_pin_write(LCD_RESET_PIN, PIN_HIGH);
}

static void hw_ili9486_lcd_init(void)
{
    lcd_reset();
    rt_pin_mode(LCD_RS_PIN, PIN_MODE_OUTPUT);
    lcd_dev = (struct rt_spi_device *)rt_device_find(LCD_DEVICE_NAME);
    if (!lcd_dev)
    {
        rt_kprintf("no %s!\n", LCD_DEVICE_NAME);
    }
    lcd_dev->config.max_hz = LCD_SPI_FREQ_MAX;//125M
    lcd_init();
}
INIT_DEVICE_EXPORT(hw_ili9486_lcd_init);
