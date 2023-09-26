#ifndef products_functions
#define products_functions

#include "pe_common.h"
#include "orders.h"
#include "trader.h"


void read_products(char * filename, exchange * ex);

void pex_print_products(exchange* ex);

product * find_matching_product(char* buffer, exchange* ex);

#endif