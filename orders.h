#ifndef orders
#define orders

#include "pe_common.h"
#include "products.h"
#include "trader.h"

// function inspired by https://stackoverflow.com/a/14770895
char* my_strdup(char* c);

int check_alphanumeric(char c);

int parse_trader_order(char* buffer, exchange* ex ,struct order * my_order, int trader_id, int order_num);

void add_order(exchange * ex, struct order order);

int cancel_order(exchange * ex, int order_id, int trader_id, struct order* deleted_order);

int amend_order(exchange * ex, int order_id, int trader_id, int price, int quantity);


void free_orders(exchange * ex);

int check_for_BUY_SELL(product *product);


void fill_order(struct order* buyer_order, struct order* seller_order, exchange* ex, int price, int product_id, int quantity, enum command fee_payer);

int match_order(exchange *ex, int product_id, struct order* latest_order);


int count_levels(product* product, enum command command);

void print_orders(exchange* ex);

void report_exchange(exchange * ex);

int check_for_active_traders(exchange* ex);

#endif