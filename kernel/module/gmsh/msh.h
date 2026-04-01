#ifndef MSHELL_H_
#define MSHELL_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

int  msh_exec(char *cmd, int length);
void msh_auto_complete(char *prefix);

int msh_exec_module(const char *cmd_line, int size);
int msh_exec_script(const char *cmd_line, int size);

#ifdef __cplusplus
}
#endif

#endif
