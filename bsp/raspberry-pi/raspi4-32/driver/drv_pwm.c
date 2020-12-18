#include <drv_pwm.h>
#include <rtthread.h>
#include <dfs_posix.h>
#include <raspi4.h>
#include "drv_gpio.h"

void pwm_init(void)
{
    prev_raspi_pin_mode(GPIO_PIN_13, ALT0);
    //prev_raspi_pin_mode(GPIO_PIN pin, GPIO_FUNC mode);
    PWM_CTL(PWM_0_BASE) = 0;
    PWM_RNG1(PWM_0_BASE) = 256;
    PWM_DAT1(PWM_0_BASE) = 128;
    PWM_CTL(PWM_0_BASE) |= PWM_CTL_PWEN1;
}

static void start_audio(void)
{
    int fd, size;
    int ii = 0;

    char testbuff[2];
    pwm_init();
    rt_kprintf("test read file\n");

    fd = open("/test2.wav", O_RDONLY);


    if (fd>= 0)
    {   
        while (size > 0)
        {
            size = read(fd, testbuff, 2);
            ii = ii + 1;
            PWM_DAT1(PWM_0_BASE) = testbuff;
            //sysPwm->dat1 = pData[i];
            //usleep_delay(27);
	        //waitMicroSeconds(27);
        }
        
        rt_kprintf("ii is %d\n", ii);
        close(fd);
    }
}
MSH_CMD_EXPORT(start_audio, audio test);
