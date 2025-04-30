#include <dev/mmcsd_spi.h>

/* These define the value returned by the MMC/SD command */

#define MMCSD_CMDRESP_R1             0
#define MMCSD_CMDRESP_R1B            1
#define MMCSD_CMDRESP_R2             2
#define MMCSD_CMDRESP_R3             3
#define MMCSD_CMDRESP_R7             4

struct mmcsd_cmdinfo_s {
    uint8_t  cmd;
    uint8_t  resp;
    uint8_t  chksum;
};


/* Commands *****************************************************************/

static const struct mmcsd_cmdinfo_s g_cmd0   =
{
    CMD0,   MMCSD_CMDRESP_R1, 0x95
};
static const struct mmcsd_cmdinfo_s g_cmd1   =
{
    CMD1,   MMCSD_CMDRESP_R1, 0xff
};
static const struct mmcsd_cmdinfo_s g_cmd8   =
{
    CMD8,   MMCSD_CMDRESP_R7, 0x87
};
static const struct mmcsd_cmdinfo_s g_cmd9   =
{
    CMD9,   MMCSD_CMDRESP_R1, 0xff
};
static const struct mmcsd_cmdinfo_s g_cmd12  =
{
    CMD12,  MMCSD_CMDRESP_R1, 0xff
};
static const struct mmcsd_cmdinfo_s g_cmd16  =
{
    CMD16,  MMCSD_CMDRESP_R1, 0xff
};
static const struct mmcsd_cmdinfo_s g_cmd17  =
{
    CMD17,  MMCSD_CMDRESP_R1, 0xff
};
static const struct mmcsd_cmdinfo_s g_cmd18  =
{
    CMD18,  MMCSD_CMDRESP_R1, 0xff
};
static const struct mmcsd_cmdinfo_s g_cmd24  =
{
    CMD24,  MMCSD_CMDRESP_R1, 0xff
};
static const struct mmcsd_cmdinfo_s g_cmd25  =
{
    CMD25,  MMCSD_CMDRESP_R1, 0xff
};
static const struct mmcsd_cmdinfo_s g_cmd55  =
{
    CMD55,  MMCSD_CMDRESP_R1, 0xff
};
static const struct mmcsd_cmdinfo_s g_cmd58  =
{
    CMD58,  MMCSD_CMDRESP_R3, 0xff
};
static const struct mmcsd_cmdinfo_s g_acmd23 =
{
    ACMD23, MMCSD_CMDRESP_R1, 0xff
};
static const struct mmcsd_cmdinfo_s g_acmd41 =
{
    ACMD41, MMCSD_CMDRESP_R1, 0xff
};
