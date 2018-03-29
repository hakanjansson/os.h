/*
 * This file is part of os.h.
 *
 * Copyright (C) 2016 Adam Heinrich <adam@adamh.cz>
 *
 * Os.h is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Os.h is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with os.h.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "project.h"
#include <os.h>

#define CM0_ISR_BASE (16)

#define ERROR_CHECK(status) \
	do { \
		if (!(status)) \
			while (1); \
	} while (0)

extern void PendSV_Handler(void);
extern void SysTick_Handler(void);

static void delay(volatile uint32_t delay_ms);
static void task_handler(void *p_params);

uint32_t SystemCoreClock = 12000000;

int main(void)
{
	bool status;

    CyIntSetSysVector( CM0_ISR_BASE + PendSV_IRQn, (cyisraddress)PendSV_Handler );
    CyIntSetSysVector( CM0_ISR_BASE + SysTick_IRQn, (cyisraddress)SysTick_Handler );

    CyGlobalIntEnable; /* Enable global interrupts. */

	/* Initialize task stacks: */
	static uint32_t stack1[128];
	static uint32_t stack2[128];
	static uint32_t stack3[128];

	/* Setup task parameters: */
	uint32_t p1 = 200000;
	uint32_t p2 = p1/2;
	uint32_t p3 = p1/4;

	status = os_init();
	ERROR_CHECK(status);

	status = os_task_init(&task_handler, (void*)p1, stack1, sizeof(stack1));
	ERROR_CHECK(status);
	status = os_task_init(&task_handler, (void*)p2, stack2, sizeof(stack2));
	ERROR_CHECK(status);
	status = os_task_init(&task_handler, (void*)p3, stack3, sizeof(stack3));
	ERROR_CHECK(status);

	/* Context switch every second: */
	status = os_start(SystemCoreClock);
	ERROR_CHECK(status);

	/* The program should never reach here: */
	while (1);
}

static void delay(volatile uint32_t time)
{
	while (time > 0)
		time--;
}

static void task_handler(void *p_params)
{
	uint32_t delay_time = (uint32_t)p_params;

	while (1) {
		__disable_irq();
		RED_LED_Write(RED_LED_ReadDataReg()^1);
		__enable_irq();

		delay(delay_time);
	}
}