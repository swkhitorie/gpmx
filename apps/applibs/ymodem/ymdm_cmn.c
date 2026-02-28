#include "ymdm_cmn.h"
#include <string.h>

static int isspace(int x)
{
    if (x == ' ' || x == '\t' || x == '\n' ||
        x == '\f' || x == '\b' || x == '\r') {
        return 1;
    }

    return 0;
}

static int isdigit(int x)
{
    if ( x <= '9' && x >= '0') {
        return 1;
	}

    return 0;
}

int y_atoi(const char* nptr)
{
    int c;              /* current char */
    int total;         /* current total */
    int sign;           /* if '-', then negative, otherwise positive */

    /* skip whitespace */
    while (isspace((int)(unsigned char)*nptr)) {
        ++nptr;
    }

    c = (int)(unsigned char)*nptr++;
    sign = c;           /* save sign indication */
    if (c == '-' || c == '+') {
        c = (int)(unsigned char)*nptr++;    /* skip sign */
    }

	total = 0;

	while (isdigit(c)) {
		total = 10 * total + (c - '0');     /* accumulate digit */
		c = (int)(unsigned char)*nptr++;    /* get next char */
	}

	if (sign == '-') {
        return -total;
	} else {
        return total;   /* return result, negated if necessary */
	}
}

int y_filename_size_get(const unsigned char *p, int len, char* fname, unsigned int *fsize)
{
    int ret=0xff;
    int i=0, j=0, k=0;
    char psize[16] = {'\0'};

	while (p[i] != 0) {
		fname[j] = p[i];
		i++;
		if (++j > len) {
			return -1;
		}
	}

	fname[j] = '\0';

	i++;
	while (p[i] != 0) {
		psize[k] = p[i];
		i++;
		if (++k > len) {
			return -2;
		}
	}
	psize[k] = '\0';

	*fsize = y_atoi(psize);

	return 0;
}

unsigned short crc16_xmodem(unsigned char *data,unsigned int datalen)
{
	unsigned short wCRCin = 0x0000;
	unsigned short wCPoly = 0x1021;
	
	while (datalen--) {
		wCRCin ^= (*(data++) << 8);
		for(int i = 0;i < 8;i++) {
			if (wCRCin & 0x8000)
				wCRCin = (wCRCin << 1) ^ wCPoly;
			else
				wCRCin = wCRCin << 1;
		}
	}
	return (wCRCin);
}


