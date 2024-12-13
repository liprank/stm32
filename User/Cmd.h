#ifndef __CMD_H__
#define __CMD_H__ 
#include "stm32f10x.h"

typedef void (*cmd_func)(void);
typedef struct { 
    const char *cmd;
    cmd_func func;
} Command;

void Start(void);
void Stop(void);
void AmplPlus(void);
void AmplMinus(void);
void FreqPlus(void);
void FreqMinus(void);

extern Command commands[7];

#endif
