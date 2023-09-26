#ifndef PE_COMMON_H
#define PE_COMMON_H

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

enum command {INVALID, BUY, SELL, AMEND, CANCEL};

typedef struct{
    int id;
    pid_t pid;
    long * balances;
    char * executable_name;
    int t_to_e;
    int e_to_t;
    int * positions;
    int active;
    int next_id;
} trader ;

typedef struct {
    int product_id;
    char name[17];
    int num_orders[3]; // note that this will be used to store the number of BUY and SELL. THe first element [0] will be empty, so that the indexes line up with the enum for commands
    struct order* buy_sell_heads[3];
    struct order* combined_orders;
} product ;

struct order {
    enum command command;
    trader * order_trader;
    int order_id;
    product * order_product;
    int quantity;
    int price;
    struct order* next_order;
    struct order* next_similar_order;
    struct order* next_combined_order;
    int order_num;
} ; 

typedef struct {
    long balance;

    trader * traders;
    int no_of_traders;

    product * products;
    int no_of_products;

    struct order * order_head;
    int no_of_orders;
} exchange;

#endif
