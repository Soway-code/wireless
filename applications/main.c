/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-03-05     whj4674672   first version
 */

#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include <Swy_nbapp.h> 

/* defined the LED0 pin: PB1 */
#define LED0_PIN    GET_PIN(B, 1)

extern int tcpclient(const char *sendBuf);

int main(void)
{
    int count = 1;
    /* set LED0 pin mode to output */	
    rt_pin_mode(LED0_PIN, PIN_MODE_OUTPUT);	
		rt_thread_mdelay(10000);
		thread_sample();

    while (1)
    {
		 // rt_thread_mdelay(10000);
		//	rt_kprintf("rtt_comein!!!\n");
		  break ;
		//	tcpclient("soway_sensor");
     /*   rt_pin_write(LED0_PIN, PIN_HIGH);
        rt_thread_mdelay(5000);
        rt_pin_write(LED0_PIN, PIN_LOW);
        rt_thread_mdelay(5000);*/
    }
    return RT_EOK;
}
