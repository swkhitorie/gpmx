#include "cli.h"

static struct cli_cmdmap_s g_cmdmap[CONFIG_CLI_MAX_COMMANDS];
static char *g_argv_list[CONFIG_CLI_MAX_COMMAND_ARGS];
static char g_clibuf[CONFIG_CLI_MAX_INPUT];
static cli_reader_t g_cli_reader = NULL;
static cli_writer_t g_cli_writer = NULL;
static bool g_cli_initialized = false;

static int  cli_parse(char * buf, char** argv, unsigned short maxargs);

static void cli_format(const char * fmt, va_list va);

static void cli_process_escape(int argc, char ** argv);

static void cli_prompt();

bool cli_register(const char *name, cli_cmd_t entry, uint8_t min, 
    uint8_t max, const char *usage)
{
    uint8_t i;
    for (i = 0; i < CONFIG_CLI_MAX_COMMANDS; i++) {
        if (g_cmdmap[i].cmd != 0 || g_cmdmap[i].handler != 0)
            continue;
        g_cmdmap[i].cmd = name;
        g_cmdmap[i].handler = entry;
        g_cmdmap[i].minargs = min;
        g_cmdmap[i].maxargs = max;
        g_cmdmap[i].usage = usage;
        return true;
	}
	return false;
}

void cli_unregister_all()
{
	uint8_t i;
	for (i = 0; i < CONFIG_CLI_MAX_COMMANDS; i++) {
		g_cmdmap[i].cmd = NULL;
		g_cmdmap[i].handler = NULL;
        g_cmdmap[i].minargs = 0;
        g_cmdmap[i].maxargs = 0;
        g_cmdmap[i].usage = NULL;
	}
}

bool cli_init(cli_reader_t reader, cli_writer_t writer, char *msg)
{
    if (reader == 0 || writer == 0)
        return false;

    cli_unregister_all();

    g_cli_reader = reader;
    g_cli_writer = writer;
    g_cli_initialized = true;

    cli_print("\r\n");
	cli_print("\r\n");
    cli_print((const char *) "FRTOS Client library Ver. ");
    cli_println((const char *) CLI_VERSION_STRING);

	cli_prompt();
	return true;
}


void cli_putc(char c)
{
    if (g_cli_initialized != false && g_cli_writer != NULL)
        g_cli_writer(c);
}

void cli_print(const char *str)
{
	while (*str != '\0')
        cli_putc(*str++);
}

void cli_println(const char *str)
{
    cli_print(str);
    cli_print("\r\n");
}

void cli_printf(const char *fmt, ...)
{
	va_list argl;
	va_start(argl, fmt);
	cli_format(fmt, argl);
	va_end(argl);
}

void cli_print_commands()
{
	uint8_t i;
	cli_println("available commands:");
	for (i = 0; i < CONFIG_CLI_MAX_COMMANDS; i++) {
		if (g_cmdmap[i].cmd != 0 || g_cmdmap[i].handler != 0) {
			cli_println(g_cmdmap[i].usage);
		}
	}
}

void cli_print_error(int error, const char *field)
{
    if (field != 0) {
        cli_print((const char *) "#ERROR-PARAM:");
        cli_print(field);
        cli_print("\r\n");
    }

    cli_print((const char *) "#ERROR-TYPE:");
    switch (error) {
    case E_CLI_ERR_ARGCOUNT:
        cli_print((const char *) "ARG-COUNT");
        break;
    case E_CLI_ERR_OUTOFRANGE:
        cli_print((const char *) "OUT-OF-RANGE");
        break;
    case E_CLI_ERR_VALUE:
        cli_print((const char *) "INVALID-VALUE");
        break;
    case E_CLI_ERR_ACTION:
        cli_print((const char *) "INVALID-ACTION");
        break;
    case E_CLI_ERR_PARSE:
        cli_print((const char *) "PARSING");
        break;
    case E_CLI_ERR_STORAGE:
        cli_print((const char *) "STORAGE");
        break;
    case E_CLI_ERR_IO:
        cli_print((const char *) "IO");
        break;
    default:
        cli_print("Unknown");
	}
    cli_print("\r\n");
}

void cli_task()
{
    static uint16_t cnt = 0;
    uint8_t i = 0;
    bool cc = 0;
    int retval = 0;
    int argc = 0;
    char ch = 0;

    if (!g_cli_initialized) return;

    if (g_cli_reader(&ch)) {
        switch (ch) {
        case CLI_ASCII_ESC: // For VT100 escape sequences
            // Process escape sequences: maybe later
            break;

        case CLI_ASCII_HT:
            cli_putc(CLI_ASCII_BEL);
            break;

        case CLI_ASCII_CR: // Enter key pressed
            g_clibuf[cnt] = '\0';
            cli_println("");
            cc = true;
            break;

        case CLI_ASCII_DEL:
        case CLI_ASCII_BS: // Backspace pressed
            if (cnt > 0) {
                cnt--;
                cli_putc(CLI_ASCII_BS);
                cli_putc(CLI_ASCII_SP);
                cli_putc(CLI_ASCII_BS);
            } else {
                cli_putc(CLI_ASCII_BEL);
            }
            break;
        default:
            // Process printable characters, but ignore other ASCII chars
            if (cnt < (CONFIG_CLI_MAX_INPUT - 1) && ch >= 0x20 && ch < 0x7F) {
                g_clibuf[cnt] = ch;
                cli_putc(ch);
                cnt++;
            }
        }

        if (cc) {
            argc = cli_parse(g_clibuf, g_argv_list, CONFIG_CLI_MAX_COMMAND_ARGS);
            cli_process_escape(argc, g_argv_list);

            for (i = 0; i < CONFIG_CLI_MAX_COMMANDS; i++) {
                if (g_cmdmap[i].handler == NULL)
                    continue;
                if (!strcmp(g_argv_list[0], g_cmdmap[i].cmd)) {
                    retval = g_cmdmap[i].handler(argc, g_argv_list);
                    cc = false;
                }
            }

            if (cc != false && cnt != 0) {
                cli_println("Command NOT found.");
            }
            cnt = 0;
            cc = false;
            cli_println("");
            cli_prompt();
        }
    }
}

static int cli_parse(char *buf, char **argv, unsigned short maxargs)
{
    int i = 0;
    int argc = 0;
    int length = strlen(buf) + 1;
    char toggle = 0;
    bool escape = false;

    argv[argc] = &buf[0];

    for (i = 0; i < length && argc < maxargs; i++) {
        switch (buf[i]) {
        case '\0':
            // Handle special char: String terminator
            i = length;
            argc++;
            break;

        case '\\':
            // Handle special char: Backslash for escape sequences
            // Begin of escape sequence, the following char will get
            // to it´s case with the boolean "escape" flag set
            escape = true;
            continue;

        case '\"':
            // Handle special char: Double quotes
            // If double quote is not escaped, process it as string start and end character
            // If it´s escaped then we do nothing and let the next step remove character escaping
            if (!escape) {
                if (toggle == 0) {
                    toggle = 1;
                    buf[i] = '\0';
                    argv[argc] = &buf[i + 1];
                } else {
                    toggle = 0;
                    buf[i] = '\0';
                }
            }
            break;

        case ' ':
        // Handle special char: Space is token separator
            if (toggle == 0) {
                buf[i] = '\0';
                argc++;
                argv[argc] = &buf[i + 1];
            }
            break;

        }
        escape = false;
    }
    return argc;
}

static void cli_process_escape(int argc, char ** argv)
{
	uint8_t i, j;
	int sl;
	uint8_t readindex = 0;

	// loop for every parameter
	for (i = 0; i < argc; i++) {
		// get the length of the current arg
		sl = strlen(argv[i]);
		// loop through every character inside this argument
		for (j = 0; j < sl; j++) {
			// search for backslash character
			if (argv[i][j + readindex] == '\\') {
				// Process escaped characters here
				if (argv[i][j + readindex + 1] == '"') {
					// Write quote double quote on the current position
					argv[i][j] = '"';
					readindex++;
				}
			}
			// read ahead and copy to current position only if escaped characters found
			if (readindex) {
				argv[i][j] = argv[i][j + readindex];
			}
		}
		// Reset for the next arg
		readindex = 0;
	}
}

static void cli_prompt()
{
	cli_print("device>");
}

#ifdef CLI_PRINTF_LONG_SUPPORT

static void uli2a(unsigned long int num, unsigned int base, int uc, char * bf)
{
	int n = 0;
	unsigned int d = 1;
	while (num / d >= base)
		d *= base;
	while (d != 0) {
		int dgt = num / d;
		num %= d;
		d /= base;
		if (n || dgt > 0 || d == 0) {
			*bf++ = dgt + (dgt < 10 ? '0' : (uc ? 'A' : 'a') - 10);
			++n;
		}
	}
	*bf = 0;
}

static void li2a(long num, char * bf)
{
	if (num < 0) {
		num = -num;
		*bf++ = '-';
	}
	uli2a(num, 10, 0, bf);
}
#endif

static void ui2a(unsigned int num, unsigned int base, int uc, char * bf)
{
	int n = 0;
	unsigned int d = 1;
	while (num / d >= base)
		d *= base;
	while (d != 0) {
		int dgt = num / d;
		num %= d;
		d /= base;
		if (n || dgt > 0 || d == 0) {
			*bf++ = dgt + (dgt < 10 ? '0' : (uc ? 'A' : 'a') - 10);
			++n;
		}
	}
	*bf = 0;
}

static void i2a(int num, char * bf)
{
	if (num < 0) {
		num = -num;
		*bf++ = '-';
	}
	ui2a(num, 10, 0, bf);
}

static int a2d(char ch)
{
	if (ch >= '0' && ch <= '9')
		return ch - '0';
	else if (ch >= 'a' && ch <= 'f')
		return ch - 'a' + 10;
	else if (ch >= 'A' && ch <= 'F')
		return ch - 'A' + 10;
	else return -1;
}

static char a2i(char ch, const char** src, int base, int* nump)
{
	const char* p = *src;
	int num = 0;
	int digit;
	while ((digit = a2d(ch)) >= 0) {
		if (digit > base) break;
		num = num * base + digit;
		ch = *p++;
	}
	*src = p;
	*nump = num;
	return ch;
}

static void putchw(int n, char z, char* bf)
{
	char fc = z ? '0' : ' ';
	char ch;
	char* p = bf;
	while (*p++ && n > 0)
		n--;
	while (n-- > 0)
		cli_putc(fc);
	while ((ch = *bf++))
		cli_putc(ch);
}

static void cli_format(const char * fmt, va_list va)
{
	char bf[12];
	char ch;


	while ((ch = *(fmt++))) {
		if (ch != '%')
			cli_putc(ch);
		else {
			char lz = 0;
#ifdef  PRINTF_LONG_SUPPORT
			char lng = 0;
#endif
			int w = 0;
			ch = *(fmt++);
			if (ch == '0') {
				ch = *(fmt++);
				lz = 1;
			}
			if (ch >= '0' && ch <= '9') {
				ch = a2i(ch, &fmt, 10, &w);
			}
#ifdef  PRINTF_LONG_SUPPORT
			if (ch == 'l') {
				ch = *(fmt++);
				lng = 1;
			}
#endif
			switch (ch) {
			case 0:
				goto abort;
			case 'u':
			{
#ifdef  PRINTF_LONG_SUPPORT
				if (lng)
					uli2a(va_arg(va, unsigned long int), 10, 0, bf);
				else
#endif
					ui2a(va_arg(va, unsigned int), 10, 0, bf);
				putchw(w, lz, bf);
				break;
			}
			case 'd':
			{
#ifdef  PRINTF_LONG_SUPPORT
				if (lng)
					li2a(va_arg(va, unsigned long int), bf);
				else
#endif
					i2a(va_arg(va, int), bf);
				putchw(w, lz, bf);
				break;
			}
			case 'x': case 'X':
#ifdef  PRINTF_LONG_SUPPORT
				if (lng)
					uli2a(va_arg(va, unsigned long int), 16, (ch == 'X'), bf);
				else
#endif
					ui2a(va_arg(va, unsigned int), 16, (ch == 'X'), bf);
				putchw(w, lz, bf);
				break;
			case 'c':
                cli_putc((char) (va_arg(va, int)));
				break;
			case 's':
				putchw(w, 0, va_arg(va, char*));
				break;
			case '%':
                cli_putc(ch);
			default:
				break;
			}
		}
	}
abort:
    ;
}

