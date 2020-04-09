/*
 * Copyright (c) 2020-2028, Soway Development Team
 *
 *
 *28 longguang Street, first Floor, China.
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-03-17     Frank        first implementation
 */


#include <board.h>
#include <rtthread.h>
#include <rtdevice.h>

#ifndef RT_USING_TIMER_SOFT
    #error "Please enable soft timer feature!"
#endif

#define TIMER_APP_DEFAULT_TICK  (RT_TICK_PER_SECOND * 2)

#ifdef RT_USING_PM 

static rt_timer_t timer1;

static void _timeout_entry(void *parameter)
{
    rt_kprintf("current tick: %ld\n", rt_tick_get());
}

static int timer_app_init(void)
{
    rt_pm_request(PM_SLEEP_MODE_IDLE);
    rt_pm_request(PM_SLEEP_MODE_LIGHT);

    timer1 = rt_timer_create("timer_app",
                             _timeout_entry,
                             RT_NULL,
                             TIMER_APP_DEFAULT_TICK,
                             RT_TIMER_FLAG_PERIODIC | RT_TIMER_FLAG_SOFT_TIMER);
    if (timer1 != RT_NULL)
    {
        rt_timer_start(timer1);

        /* keep in timer mode */
        rt_pm_request(PM_SLEEP_MODE_DEEP);
     rt_kprintf("rt_pm_request comin\n");
        return 0;
    }
    else
    {
        return -1;
    }
}
//INIT_APP_EXPORT(timer_app_init);
#endif
