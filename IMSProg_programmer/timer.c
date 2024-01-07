/*
 * Copyright (C) 2021 McMCC <mcmcc@mail.ru>
 * timer.c
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <stdio.h>
#include <time.h>

#include "timer.h"

static time_t start_time = 0;
static time_t print_time = 0;

void timer_start(void)
{
	start_time = time(0);
}

void timer_end(void)
{
	time_t end_time = 0, elapsed_seconds = 0;

	time(&end_time);
	elapsed_seconds = difftime(end_time, start_time);
	printf("Elapsed time: %d seconds\n", (int)elapsed_seconds);
	print_time = 0;
}

int timer_progress(void)
{
	time_t end_time = 0;
	int elapsed_seconds = 0;

	if (!print_time)
		print_time = time(0);

	time(&end_time);

	elapsed_seconds = (int)difftime(end_time, print_time);

	if (elapsed_seconds == 1) {
		print_time = 0;
		return 1;
	}
	return 0;
}
/* End of [timer.c] package */
