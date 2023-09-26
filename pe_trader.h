#ifndef PE_TRADER_H
#define PE_TRADER_H

// #include "pe_common.h"
#define _POSIX_SOURCE
#define _POSIX_C_SOURCE 199309L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <math.h>


#define FIFO_EXCHANGE "/tmp/pe_exchange_%d"
#define FIFO_TRADER "/tmp/pe_trader_%d"
#define FEE_PERCENTAGE 1

#define MARKET_OPEN_MESSAGE "MARKET OPEN;"
#define MARKET_SELL_MESSAGE "MARKET SELL %s %d %d;"
#define MARKET_BUY_MESSAGE "MARKET BUY %s %d %d;"
// #define MARKET_AMEND_MESSAGE "AMEND %d %d %d"
// #define MARKET_CANCEL_MESSAGE "CANCEL %d %d %d"

#define MAX_COMMAND_LENGTH 50


// void handler(int signo, siginfo_t* sinfo, void* context);

#endif
