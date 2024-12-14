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
 * @file   : task_normal.c
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
#include "string.h"

/* Demo includes. */
#include "logger.h"
#include "dwt.h"

/* Application & Tasks includes. */
#include "board.h"
#include "app.h"
#include "task_actuator_attribute.h"
#include "task_actuator_interface.h"
// #include "task_temperature.h"
// #include "display.h"

/********************** macros and definitions *******************************/
#define G_TASK_SYS_CNT_INI			0ul
#define G_TASK_SYS_TICK_CNT_INI		0ul

#define DEL_NML_MIN_SPEED          	1ul

#define DEL_NML_DEF_SPEED			10ul
#define DEL_NML_DEF_PACK_RATE		2ul
#define DEL_NML_DEF_WAITING_TIME	5ul

#define DEL_NML_MAX_SPEED			20ul

/********************** internal data declaration ****************************/
task_normal_dta_t task_normal_dta =
	{DEL_SYST_MIN, DEL_SYST_MIN, DEL_SYST_MIN, ST_NML_IDLE, EV_NML_SYST_CTRL_OFF, false};

#define SYSTEM_DTA_QTY	(sizeof(task_normal_dta)/sizeof(task_normal_dta_t))

/********************** internal functions declaration ***********************/

/********************** internal data definition *****************************/
const char *p_task_normal 		= "Task Normal (System Statechart)";
const char *p_task_normal_ 		= "Non-Blocking & Update By Time Code";

/********************** external data declaration ****************************/
uint32_t g_task_normal_cnt;
volatile uint32_t g_task_normal_tick_cnt;

/********************** external functions definition ************************/
void task_normal_init(void *parameters) {
	task_normal_dta_t 			*p_task_normal_dta;
	task_normal_st_t			state;
	task_normal_ev_t			event;
	bool 						b_event;
	task_shared_params_dta_t *p_task_shared_params_dta = (task_shared_params_dta_t *)parameters;

	/* Print out: Task Initialized */
	LOGGER_LOG("  %s is running - %s\r\n", GET_NAME(task_normal_init), p_task_normal);
	LOGGER_LOG("  %s is a %s\r\n", GET_NAME(task_normal), p_task_normal_);

	g_task_normal_cnt = G_TASK_SYS_CNT_INI;

	/* Print out: Task execution counter */
	LOGGER_LOG("   %s = %lu\r\n", GET_NAME(g_task_normal_cnt), g_task_normal_cnt);

	init_queue_event_task_normal();

	/* Update Task Actuator Configuration & Data Pointer */
	p_task_normal_dta = &task_normal_dta;

	/* Print out: Task execution FSM */
	state = p_task_normal_dta->state;
	LOGGER_LOG("   %s = %lu", GET_NAME(state), (uint32_t)state);

	event = p_task_normal_dta->event;
	LOGGER_LOG("   %s = %lu", GET_NAME(event), (uint32_t)event);

	b_event = p_task_normal_dta->flag;
	LOGGER_LOG("   %s = %s\r\n", GET_NAME(b_event), (b_event ? "true" : "false"));

	g_task_normal_tick_cnt = G_TASK_SYS_TICK_CNT_INI;

	//displayInit();
}

void task_normal_update(void *parameters) {
	task_normal_dta_t *p_task_normal_dta;
	bool b_time_update_required = false;
	task_shared_params_dta_t *p_task_shared_params_dta = (task_shared_params_dta_t *)parameters;

	/* Update Task System Counter */
	g_task_normal_cnt++;

	/* Protect shared resource (g_task_normal_tick) */
	__asm("CPSID i");	/* disable interrupts*/
    if (G_TASK_SYS_TICK_CNT_INI < g_task_normal_tick_cnt) {
    	g_task_normal_tick_cnt--;
    	b_time_update_required = true;
    }
    __asm("CPSIE i");	/* enable interrupts*/

    while (b_time_update_required) {
		/* Protect shared resource (g_task_normal_tick) */
		__asm("CPSID i");	/* disable interrupts*/
		if (G_TASK_SYS_TICK_CNT_INI < g_task_normal_tick_cnt) {
			g_task_normal_tick_cnt--;
			b_time_update_required = true;
		}
		else {
			b_time_update_required = false;
		}
		__asm("CPSIE i");	/* enable interrupts*/

    	/* Update Task System Data Pointer */
		p_task_normal_dta = &task_normal_dta;

		if (true == any_event_task_normal()) {
			p_task_normal_dta->flag = true;
			p_task_normal_dta->event = get_event_task_normal();
		}

		switch (p_task_normal_dta->state) {

			case ST_NML_IDLE:

				if (EV_NML_SYST_CTRL_ON == p_task_normal_dta->event) {
					LOGGER_LOG("ENTRE AL SISTEMA DE CONTROL");
					p_task_normal_dta->state = ST_NML_SYST_CTRL;
					p_task_normal_dta->qty_packs = DEL_SYST_MIN;
					p_task_normal_dta->speed = DEL_NML_DEF_SPEED;
					p_task_shared_params_dta->pack_rate = DEL_NML_DEF_PACK_RATE;
					p_task_shared_params_dta->waiting_time = DEL_NML_DEF_WAITING_TIME;
					p_task_normal_dta->tick = DEL_SYST_MIN;
				}

				if (EV_NML_SETUP_ON == p_task_normal_dta->event) {
					LOGGER_LOG("ENTRE AL SETUP");
					p_task_normal_dta->state = ST_NML_SETUP;
					p_task_normal_dta->qty_packs = DEL_SYST_MIN;
					p_task_normal_dta->speed = DEL_NML_DEF_SPEED;
					p_task_shared_params_dta->pack_rate = DEL_NML_DEF_PACK_RATE;
					p_task_shared_params_dta->waiting_time = DEL_NML_DEF_WAITING_TIME;
					p_task_normal_dta->tick = DEL_SYST_MIN;
					put_event_task_setup(EV_SETUP_ON);
				}

				break;

			case ST_NML_SYST_CTRL:


				/*if (p_task_normal_dta->qty_packs == DEL_SYST_MIN)
					put_event_task_actuator(EV_LED_XX_TURN_ON, ID_LED_MIN_SPEED);

				if (p_task_normal_dta->qty_packs == DEL_SYST_MAX_PACKS)
					put_event_task_actuator(EV_LED_XX_TURN_ON, ID_LED_MAX_SPEED);*/

				if (EV_NML_PACK_IN == p_task_normal_dta->event && p_task_normal_dta->qty_packs < DEL_SYST_MAX_PACKS) {
					LOGGER_LOG("SUBE LA CANT PACKS SIN BAJAR VEL\n");
					p_task_normal_dta->qty_packs++;
				}

				else if (EV_NML_PACK_IN == p_task_normal_dta->event && (p_task_normal_dta->qty_packs % p_task_shared_params_dta->pack_rate) == 0
						&& p_task_normal_dta->speed > DEL_NML_MIN_SPEED && p_task_normal_dta->qty_packs < DEL_SYST_MAX_PACKS) {
					LOGGER_LOG("SUBE LA CANT PACKS BAJANDO VEL\n");
					p_task_normal_dta->speed--;
					p_task_normal_dta->qty_packs++;
				}

				if (EV_NML_NO_PACKS == p_task_normal_dta->event && p_task_normal_dta->tick == p_task_shared_params_dta->waiting_time
						&& p_task_normal_dta->qty_packs == DEL_SYST_MIN) {
					LOGGER_LOG("NO HAY PACKS Y SE CUMPLIÓ EL TIEMPO DE ESPERA\n");
					put_event_task_normal(EV_NML_SYST_CTRL_OFF);
				}

				else if (EV_NML_NO_PACKS == p_task_normal_dta->event && p_task_normal_dta->qty_packs == DEL_SYST_MIN) {
					LOGGER_LOG("AUMENTA TIEMPO DE ESPERA SI NO HAY PACKS\n");
					p_task_normal_dta->tick++;
				}

				if (EV_NML_PACK_OUT == p_task_normal_dta->event && p_task_normal_dta->qty_packs > DEL_SYST_MIN) {
					LOGGER_LOG("BAJA LA CANT PACKS SIN SUBIR VEL\n");
					p_task_normal_dta->qty_packs--;
				}

				else if (EV_NML_PACK_OUT == p_task_normal_dta->event && (p_task_normal_dta->qty_packs % p_task_shared_params_dta->pack_rate) == 0
						&& p_task_normal_dta->speed < DEL_NML_MAX_SPEED && p_task_normal_dta->qty_packs > DEL_SYST_MIN) {
					LOGGER_LOG("BAJA LA CANT PACKS SUBIENDO VEL\n");
					p_task_normal_dta->speed++;
					p_task_normal_dta->qty_packs--;
				}

				if (EV_NML_SETUP_ON == p_task_normal_dta->event) {
					LOGGER_LOG("ESTOY EN EL SETUP\n");
					p_task_normal_dta->state = ST_NML_SETUP;
					put_event_task_setup(EV_SETUP_ON);
				}

				if (EV_NML_SYST_CTRL_OFF == p_task_normal_dta->event) {
					LOGGER_LOG("SE APAGA EL SYST DE CONTROL\n");
					p_task_normal_dta->state = ST_NML_IDLE;
					p_task_normal_dta->qty_packs = DEL_SYST_MIN;
					p_task_normal_dta->speed = DEL_SYST_MIN;
					p_task_shared_params_dta->pack_rate = DEL_SYST_MIN;
					p_task_shared_params_dta->waiting_time = DEL_SYST_MIN;
					p_task_normal_dta->tick = DEL_SYST_MIN;
				}

				break;

			case ST_NML_SETUP:

				if (EV_NML_SETUP_OFF == p_task_normal_dta->event) {
					LOGGER_LOG("SE APAGA EL SETUP\n");
					p_task_normal_dta->state = ST_NML_SYST_CTRL;
					put_event_task_setup(EV_SETUP_OFF);
				}

				break;
		}
    }
}

/********************** end of file ******************************************/
