/*
 * task_shared_params.h
 *
 *  Created on: Dec 14, 2024
 *      Author: Usuario
 */

#ifndef INC_TASK_SHARED_PARAMS_H_
#define INC_TASK_SHARED_PARAMS_H_

#include <stdint.h>

/********************** macros and definitions *******************************/
#define DEL_SYST_MIN	 			0ul
#define DEL_SYST_MIN_PACK_RATE		1ul
#define DEL_SYST_MIN_WAITING_TIME	1ul

#define DEL_SYST_MAX_PACKS			10ul
#define DEL_SYST_MAX_WAITING_TIME	30ul

/********************** typedef **********************************************/

typedef struct
{
	uint32_t		pack_rate;
	uint32_t		waiting_time;
} task_shared_params_dta_t;

/********************** external data declaration ****************************/
extern task_shared_params_dta_t task_shared_params_dta;

#endif /* INC_TASK_SHARED_PARAMS_H_ */
