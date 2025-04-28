#ifndef CLI__H_
#define CLI__H_

#include <string.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdbool.h>

#if !defined(CONFIG_CLI_MAX_COMMANDS)
#define CONFIG_CLI_MAX_COMMANDS      (20)
#endif

#if !defined(CONFIG_CLI_MAX_COMMAND_ARGS)
#define CONFIG_CLI_MAX_COMMAND_ARGS		(10)
#endif

#if !defined(CONFIG_CLI_MAX_INPUT)
#define CONFIG_CLI_MAX_INPUT			(70)
#endif

#define CLI_ASCII_NUL               0x00
#define CLI_ASCII_BEL               0x07
#define CLI_ASCII_BS                0x08
#define CLI_ASCII_HT                0x09
#define CLI_ASCII_LF                0x0A
#define CLI_ASCII_CR                0x0D
#define CLI_ASCII_ESC               0x1B
#define CLI_ASCII_DEL               0x7F
#define CLI_ASCII_US                0x1F
#define CLI_ASCII_SP                0x20
#define CLI_VT100_ARROWUP           'A'
#define CLI_VT100_ARROWDOWN         'B'
#define CLI_VT100_ARROWRIGHT        'C'
#define CLI_VT100_ARROWLEFT         'D'

#define CLI_RET_SUCCESS             0
#define CLI_RET_FAILURE             1
#define CLI_RET_IOPENDING           -1

#define CLI_VERSION_STRING          "1.0.0"


/*  definition for all the programs invoked by the shell (function pointer) */
typedef int  (*cli_cmd_t)(int argc, char **argv);
typedef void (*cli_writer_t) (char);
typedef int  (*cli_reader_t) (char*);
typedef void (*cli_bwriter_t)(char *, uint8_t);

enum cli_errors {
	/** There are missing arguments for the command */
	E_CLI_ERR_ARGCOUNT = 0,
	/** The program received an argument that is out of range */
	E_CLI_ERR_OUTOFRANGE,
	/** The program received an argument with a value different than expected */
	E_CLI_ERR_VALUE,
	/** Invalid action requested for the current state */
	E_CLI_ERR_ACTION,
	/** Cannot parse the user input */
	E_CLI_ERR_PARSE,
	/** Cannot access storage device or memory device */
	E_CLI_ERR_STORAGE,
	/** IO device error caused program interruption */
	E_CLI_ERR_IO,
	/** Other kinds of errors */
	E_CLI_ERROR_UNKNOWN,
};

struct cli_cmdmap_s {
    const char *cmd;    /* Name of the command */
    cli_cmd_t   handler;
    uint8_t     minargs;
    uint8_t     maxargs;
    const char *usage;  /* Usage instructions for 'help' command */
};

#ifdef __cplusplus
extern "C" {
#endif

    bool cli_init(cli_reader_t reader, cli_writer_t writer, char *msg);

    void cli_unregister_all();

    bool cli_register(const char *name, cli_cmd_t entry, uint8_t min, 
		uint8_t max, const char *usage);

    void cli_putc(char c);
    void cli_print(const char *str);
    void cli_println(const char *str);
    void cli_printf(const char *fmt, ...);
    void cli_print_commands();
    void cli_print_error(int error, const char *field);

    void cli_task();

#ifdef __cplusplus
}
#endif


#endif
