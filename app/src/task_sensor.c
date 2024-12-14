/*
 * Copyright (c) 2023 Juan Manuel Cruz <jcruz@fi.uba.ar> <jcruz@frba.utn.edu.ar>.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 *
 * @file   : task_sensor.c
 * @date   : Set 26, 2023
 * @author : Juan Manuel Cruz <jcruz@fi.uba.ar> <jcruz@frba.utn.edu.ar>
 * @version	v1.0.0
 */

/********************** inclusions *******************************************/
/* Project includes. */
#include "task_normal_attribute.h"
#include "task_normal_interface.h"
#include "task_setup_attribute.h"
#include "task_setup_interface.h"
#include "main.h"

/* Demo includes. */
#include "logger.h"
#include "dwt.h"

/* Application & Tasks includes. */
#include "board.h"
#include "app.h"
#include "task_sensor_attribute.h"

/********************** macros and definitions *******************************/
#define G_TASK_SEN_CNT_INIT			0ul
#define G_TASK_SEN_TICK_CNT_INI		0ul

#define DEL_BTN_01_MIN				0ul
#define DEL_BTN_01_MED				25ul
#define DEL_BTN_01_MAX				50ul

/********************** internal data declaration ****************************/
const task_sensor_cfg_t task_sensor_cfg_list[] = {
	{ID_BTN_PACK_IN,  BTN_PACK_IN_PORT,  BTN_PACK_IN_PIN,  BTN_PACK_IN_PRESSED, DEL_BTN_01_MAX,
	 EV_NML_NO_PACK_IN,  EV_NML_PACK_IN},
	{ID_BTN_PACK_OUT,  BTN_PACK_OUT_PORT,  BTN_PACK_OUT_PIN,  BTN_PACK_OUT_PRESSED, DEL_BTN_01_MAX,
	 EV_NML_NO_PACK_OUT,  EV_NML_PACK_OUT},
	{ID_DIP_NORMAL_OR_SETUP,  DIP_NORMAL_OR_SETUP_PORT,  DIP_NORMAL_OR_SETUP_PIN,  DIP_NORMAL_OR_SETUP_PRESSED, DEL_BTN_01_MAX,
	 EV_NML_SETUP_OFF,  EV_NML_SETUP_ON},
	{ID_DIP_INFRARED,  DIP_INFRARED_PORT,  DIP_INFRARED_PIN,  DIP_INFRARED_PRESSED, DEL_BTN_01_MAX,
	 EV_NML_PACKS,  EV_NML_NO_PACKS},
	{ID_DIP_CTRL_SYST_ON,  DIP_CTRL_SYST_PORT,  DIP_CTRL_SYST_PIN,  DIP_CTRL_SYST_PRESSED, DEL_BTN_01_MAX,
	 EV_NML_SYST_CTRL_OFF,  EV_NML_SYST_CTRL_ON},
	{ID_BTN_ENTER,  BTN_SETUP_ENTER_PORT,  BTN_SETUP_ENTER_PIN,  BTN_SETUP_ENTER_PRESSED, DEL_BTN_01_MAX,
	 EV_SETUP_IDLE,  EV_SETUP_ENTER},
	{ID_BTN_NEXT,  BTN_SETUP_NEXT_PORT,  BTN_SETUP_NEXT_PIN,  BTN_SETUP_NEXT_PRESSED, DEL_BTN_01_MAX,
	 EV_SETUP_IDLE,  EV_SETUP_NEXT},
	{ID_BTN_ESCAPE,  BTN_SETUP_ESCAPE_PORT,  BTN_SETUP_ESCAPE_PIN,  BTN_SETUP_ESCAPE_PRESSED, DEL_BTN_01_MAX,
     EV_SETUP_IDLE,  EV_SETUP_ESCAPE}
};

#define SENSOR_CFG_QTY	(sizeof(task_sensor_cfg_list)/sizeof(task_sensor_cfg_t))

task_sensor_dta_t task_sensor_dta_list[] = {
	{DEL_BTN_01_MIN, ST_BTN_01_UP, EV_BTN_01_NOT_PRESSED},
	{DEL_BTN_01_MIN, ST_BTN_01_UP, EV_BTN_01_NOT_PRESSED},
	{DEL_BTN_01_MIN, ST_BTN_01_UP, EV_BTN_01_NOT_PRESSED},
	{DEL_BTN_01_MIN, ST_BTN_01_UP, EV_BTN_01_NOT_PRESSED},
	{DEL_BTN_01_MIN, ST_BTN_01_UP, EV_BTN_01_NOT_PRESSED},
	{DEL_BTN_01_MIN, ST_BTN_01_UP, EV_BTN_01_NOT_PRESSED},
	{DEL_BTN_01_MIN, ST_BTN_01_UP, EV_BTN_01_NOT_PRESSED},
	{DEL_BTN_01_MIN, ST_BTN_01_UP, EV_BTN_01_NOT_PRESSED}
};

#define SENSOR_DTA_QTY	(sizeof(task_sensor_dta_list)/sizeof(task_sensor_dta_t))

/********************** internal functions declaration ***********************/

/********************** internal data definition *****************************/
const char *p_task_sensor 		= "Task Sensor (Sensor Statechart)";
const char *p_task_sensor_ 		= "Non-Blocking & Update By Time Code";

/********************** external data declaration ****************************/
uint32_t g_task_sensor_cnt;
volatile uint32_t g_task_sensor_tick_cnt;

/********************** external functions definition ************************/
void task_sensor_init(void *parameters)
{
	uint32_t index;
	task_sensor_dta_t *p_task_sensor_dta;
	task_sensor_st_t state;
	task_sensor_ev_t event;

	/* Print out: Task Initialized */
	LOGGER_LOG("  %s is running - %s\r\n", GET_NAME(task_sensor_init), p_task_sensor);
	LOGGER_LOG("  %s is a %s\r\n", GET_NAME(task_sensor), p_task_sensor_);

	g_task_sensor_cnt = G_TASK_SEN_CNT_INIT;

	/* Print out: Task execution counter */
	LOGGER_LOG("   %s = %lu\r\n", GET_NAME(g_task_sensor_cnt), g_task_sensor_cnt);

	for (index = 0; SENSOR_DTA_QTY > index; index++)
	{
		/* Update Task Sensor Data Pointer */
		p_task_sensor_dta = &task_sensor_dta_list[index];

		/* Print out: Index & Task execution FSM */
		LOGGER_LOG("   %s = %lu", GET_NAME(index), index);

		state = p_task_sensor_dta->state;
		LOGGER_LOG("   %s = %lu", GET_NAME(state), (uint32_t)state);

		event = p_task_sensor_dta->event;
		LOGGER_LOG("   %s = %lu\r\n", GET_NAME(event), (uint32_t)event);
	}
	g_task_sensor_tick_cnt = G_TASK_SEN_TICK_CNT_INI;
}

void task_sensor_update(void *parameters)
{
	uint32_t index;
	const task_sensor_cfg_t *p_task_sensor_cfg;
	task_sensor_dta_t *p_task_sensor_dta;
	bool b_time_update_required = false;

	/* Update Task Sensor Counter */
	g_task_sensor_cnt++;

	/* Protect shared resource (g_task_sensor_tick_cnt) */
	__asm("CPSID i");	/* disable interrupts*/
    if (G_TASK_SEN_TICK_CNT_INI < g_task_sensor_tick_cnt)
    {
    	g_task_sensor_tick_cnt--;
    	b_time_update_required = true;
    }
    __asm("CPSIE i");	/* enable interrupts*/

    while (b_time_update_required)
    {
		/* Protect shared resource (g_task_sensor_tick_cnt) */
		__asm("CPSID i");	/* disable interrupts*/
		if (G_TASK_SEN_TICK_CNT_INI < g_task_sensor_tick_cnt)
		{
			g_task_sensor_tick_cnt--;
			b_time_update_required = true;
		}
		else
		{
			b_time_update_required = false;
		}
		__asm("CPSIE i");	/* enable interrupts*/

    	for (index = 0; SENSOR_DTA_QTY > index; index++)
		{
    		/* Update Task Sensor Configuration & Data Pointer */
			p_task_sensor_cfg = &task_sensor_cfg_list[index];
			p_task_sensor_dta = &task_sensor_dta_list[index];

			if (p_task_sensor_cfg->pressed == HAL_GPIO_ReadPin(p_task_sensor_cfg->gpio_port, p_task_sensor_cfg->pin))
			{
				p_task_sensor_dta->event =	EV_BTN_01_PRESSED;
			}
			else
			{
				p_task_sensor_dta->event =	EV_BTN_01_NOT_PRESSED;
			}

			switch (p_task_sensor_dta->state)
			{
				case ST_BTN_01_UP:

					if (EV_BTN_01_PRESSED == p_task_sensor_dta->event)
					{
						p_task_sensor_dta->state = ST_BTN_01_FALLING;
						p_task_sensor_dta->tick = DEL_BTN_01_MAX;
					}

					break;

				case ST_BTN_01_FALLING:

					if (EV_BTN_01_PRESSED == p_task_sensor_dta->event)
					{
						if (p_task_sensor_dta->tick > 0)
						{
							p_task_sensor_dta->tick--;
						}

						else
						{
							put_event_task_normal(p_task_sensor_cfg->signal_down);
							put_event_task_setup(p_task_sensor_cfg->signal_down);
							p_task_sensor_dta->state = ST_BTN_01_DOWN;
						}
					}

					if (EV_BTN_01_NOT_PRESSED == p_task_sensor_dta->event)
					{
						p_task_sensor_dta->state = ST_BTN_01_UP;
						p_task_sensor_dta->tick = 0;
					}

					break;

				case ST_BTN_01_DOWN:

					if (EV_BTN_01_NOT_PRESSED == p_task_sensor_dta->event)
					{
						p_task_sensor_dta->state = ST_BTN_01_INCREASING;
						p_task_sensor_dta->tick = DEL_BTN_01_MAX;
					}

					break;

				case ST_BTN_01_INCREASING:

					if (EV_BTN_01_NOT_PRESSED == p_task_sensor_dta->event)
					{
						if (p_task_sensor_dta->tick > 0)
						{
							p_task_sensor_dta->tick--;
						}

						else
						{
							put_event_task_normal(p_task_sensor_cfg->signal_up);
							put_event_task_setup(p_task_sensor_cfg->signal_up);
							p_task_sensor_dta->state = ST_BTN_01_UP;
						}
					}

					if (EV_BTN_01_PRESSED == p_task_sensor_dta->event)
					{
						p_task_sensor_dta->state = ST_BTN_01_DOWN;
					}

					break;
			}
		}
    }
}

/********************** end of file ******************************************/
