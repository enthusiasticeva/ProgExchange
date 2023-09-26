#include "pe_common.h"
#include "orders.h"
#include "trader.h"



void read_products(char * filename, exchange * ex) {
    FILE* product_file = fopen(filename, "r");

    if (product_file == NULL) {
        perror("Unable to open file");
    }

    char line[20];
    fgets(line, 20, product_file);

    ex -> no_of_products = atoi(line);

    //allocate enough space to hold all products
    ex -> products = (product *) malloc(ex -> no_of_products * sizeof(product));
    if (ex -> products == NULL) {
        perror("malloc was unsuccessful");
    }

    int i = 0;
    while (i < ex -> no_of_products) {
        fgets(line, 20, product_file);
        product temp_product;
        
        //remove the newline, add a null char on the end
        int x = 0;
        while (x < 16 && check_alphanumeric(line[x])) {
            temp_product.name[x] = line[x];
            x ++;
        }
        temp_product.product_id = i;
        temp_product.name[x] = 0;
        temp_product.num_orders[0] = 0;
        temp_product.num_orders[BUY] = 0;
        temp_product.num_orders[SELL] = 0;
        temp_product.buy_sell_heads[BUY] = NULL;
        temp_product.buy_sell_heads[SELL] = NULL;
        temp_product.combined_orders = NULL;


        ex -> products[i] = temp_product;
        i ++;
    }

    fclose(product_file);
}

void pex_print_products(exchange* ex) {
    printf("[PEX] Trading %d products: ", ex->no_of_products);
    int p = 0;
    while (p < ex->no_of_products) {
        printf("%s", ex->products[p].name);
        if (p < ex->no_of_products - 1) {
            printf(" ");
        }
        p ++;
    }
    printf("\n");
}

product * find_matching_product(char* buffer, exchange* ex) {
    int i = 0;
    while (i < ex->no_of_products) {
        if (strcmp(ex->products[i].name, buffer) == 0) {
            return  &(ex->products[i]);
        }
        i ++;
    }
    return NULL;
}