/****************************************************************************
 *
 * Copyright (C) 2017-2018 PX4 Development Team. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name PX4 nor the names of its contributors may be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************/

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <common/log.h>
#include <drivers/drv_hrt.h>

__EXPORT const char *__px4_log_level_str[_PX4_LOG_LEVEL_PANIC + 1] = { "DEBUG", "INFO", "WARN", "ERROR", "PANIC" };
static constexpr const char *__px4_log_level_color[_PX4_LOG_LEVEL_PANIC + 1] {
	PX4_ANSI_COLOR_GREEN,  // DEBUG
	PX4_ANSI_COLOR_RESET,  // INFO
	PX4_ANSI_COLOR_YELLOW, // WARN
	PX4_ANSI_COLOR_RED,    // ERROR
	PX4_ANSI_COLOR_RED     // PANIC
};

__EXPORT void px4_log_modulename(int level, const char *module_name, const char *fmt, ...)
{
	FILE *out = stdout;
	bool use_color = true;

#ifndef PX4_LOG_COLORIZED_OUTPUT
	use_color = false;
#endif

	if (level >= _PX4_LOG_LEVEL_INFO) {
		if (use_color) { fputs(__px4_log_level_color[level], out); }

		fprintf(out, __px4__log_level_fmt __px4__log_level_arg(level));

		if (use_color) { fputs(PX4_ANSI_COLOR_GRAY, out); }

		fprintf(out, __px4__log_modulename_pfmt, module_name);

		if (use_color) { fputs(__px4_log_level_color[level], out); }

		va_list argptr;
		va_start(argptr, fmt);
		vfprintf(out, fmt, argptr);
		va_end(argptr);

		if (use_color) { fputs(PX4_ANSI_COLOR_RESET, out); }

		fputc('\n', out);
	}
}

__EXPORT void px4_log_raw(int level, const char *fmt, ...)
{
	FILE *out = stdout;
	bool use_color = true;

#ifndef PX4_LOG_COLORIZED_OUTPUT
	use_color = false;
#endif

	if (level >= _PX4_LOG_LEVEL_INFO) {
		if (use_color) { fputs(__px4_log_level_color[level], out); }

		va_list argptr;
		va_start(argptr, fmt);
		vfprintf(out, fmt, argptr);
		va_end(argptr);

		if (use_color) { fputs(PX4_ANSI_COLOR_RESET, out); }
	}
}
