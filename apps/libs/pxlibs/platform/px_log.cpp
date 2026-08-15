#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef MODULE_NAME
#define MODULE_NAME "log"
#endif

#include <platform_common/log.h>

#include <libs/mathlib/mathlib.h>
#include <uorb/topics/log_message.h>
#include <uorb/uorb_common.h>

static orb_advert_t orb_log_message_pub = nullptr;
const char *__px4_log_level_str[_PX4_LOG_LEVEL_PANIC + 1] = { "DEBUG", "INFO", "WARN", "ERROR", "PANIC" };

static constexpr const char *__px4_log_level_color[_PX4_LOG_LEVEL_PANIC + 1] {
	PX4_ANSI_COLOR_GREEN,  // DEBUG
	PX4_ANSI_COLOR_RESET,  // INFO
	PX4_ANSI_COLOR_YELLOW, // WARN
	PX4_ANSI_COLOR_RED,    // ERROR
	PX4_ANSI_COLOR_RED     // PANIC
};

static void (*px4_log_streamout)(const char *buf);

void px4_log_initialize(void)
{
	// we need to advertise with a valid message
	log_message_s log_message{};
	log_message.severity = 6; // info
	strcpy((char *)log_message.text, "initialized uORB logging");
	log_message.timestamp = hrt_absolute_time();
	orb_log_message_pub = orb_advertise_queue(ORB_ID(log_message), &log_message, log_message_s::ORB_QUEUE_LENGTH);
}

extern "C" void px4_log_streamouthook_set(void (*out)(const char *buf))
{
	px4_log_streamout = out;
}

extern "C" void px4_log_modulename(int level, const char *module_name, const char *fmt, ...)
{
	static constexpr ssize_t max_length = sizeof(log_message_s::text);

	if (level >= _PX4_LOG_LEVEL_INFO) {
		char buf[max_length + 1]; // same length as log_message_s::text, but add newline
		ssize_t pos = 0;

		pos += snprintf(buf + pos, math::max(max_length - pos, (ssize_t)0), __px4__log_level_fmt, __px4_log_level_str[level]);

		pos += snprintf(buf + pos, math::max(max_length - pos, (ssize_t)0), __px4__log_modulename_pfmt, module_name);

		va_list argptr;
		va_start(argptr, fmt);
		pos += vsnprintf(buf + pos, math::max(max_length - pos, (ssize_t)0), fmt, argptr);
		va_end(argptr);

		pos += sprintf(buf + math::min(pos, max_length - (ssize_t)1), "\n");

		// ensure NULL termination (buffer is max_length + 1)
		buf[max_length] = 0;

		
		if (px4_log_streamout) {
			px4_log_streamout(buf);
		}
	}

	/* publish an orb log message */
	if (level >= _PX4_LOG_LEVEL_INFO && orb_log_message_pub) { //publish all messages

		log_message_s log_message;
		const uint8_t log_level_table[] = {
			7, /* _PX4_LOG_LEVEL_DEBUG */
			6, /* _PX4_LOG_LEVEL_INFO */
			4, /* _PX4_LOG_LEVEL_WARN */
			3, /* _PX4_LOG_LEVEL_ERROR */
			0  /* _PX4_LOG_LEVEL_PANIC */
		};
		log_message.severity = log_level_table[level];

		ssize_t pos = snprintf((char *)log_message.text, max_length, __px4__log_modulename_pfmt, module_name);

		va_list argptr;
		va_start(argptr, fmt);
		pos += vsnprintf((char *)log_message.text + pos, math::max(max_length - pos, (ssize_t)0), fmt, argptr);
		va_end(argptr);
		log_message.text[max_length - 1] = 0; //ensure 0-termination
		log_message.timestamp = hrt_absolute_time();
		orb_publish(ORB_ID(log_message), orb_log_message_pub, &log_message);
	}
}

extern "C" void px4_log_raw(int level, const char *fmt, ...)
{
	if (level >= _PX4_LOG_LEVEL_INFO) {
		static constexpr ssize_t max_length = sizeof(log_message_s::text);
		char buf[max_length + 1]; // same length as log_message_s::text, but add newline
		ssize_t pos = 0;

		va_list argptr;
		va_start(argptr, fmt);
		pos += vsnprintf(buf + pos, math::max(max_length - pos, (ssize_t)0), fmt, argptr);
		va_end(argptr);

		if (pos > max_length) {
			// preserve newline if necessary
			if (fmt[strlen(fmt) - 1] == '\n') {
				buf[max_length - 1] = '\n';
			}
		}

		// ensure NULL termination (buffer is max_length + 1)
		buf[max_length] = 0;

		if (px4_log_streamout) {
			px4_log_streamout(buf);
		}
	}
}
