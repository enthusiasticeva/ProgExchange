#ifndef trader_functions
#define trader_functions

#include "pe_common.h"
#include "products.h"
#include "orders.h"

void send_message_to_trader(exchange* ex, int trader, char * message);

void send_message_to_exchange(int pipe, char * message);

void get_message(int file_to_read, char * buffer);

int get_trader_id_from_pid(exchange* ex, int pid);

void print_trader(exchange* ex, trader* trader);

void print_traders(exchange* ex);



#endif