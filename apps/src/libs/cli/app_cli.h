#ifndef APP_CLI_H_
#define APP_CLI_H_

#include "cli.h"

#ifdef CONFIG_FRCLI_EXCMD_NUM
extern const struct cli_cmdmap_s ex_cmdmap[CONFIG_FRCLI_EXCMD_NUM];
#endif

#ifdef __cplusplus
extern "C" {
#endif

void fcli_init();

#ifdef __cplusplus
}
#endif

#endif
