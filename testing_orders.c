#include "pe_common.h"
#include "orders.h"


int main(int argc, char ** argv) {
    //check for enough arguments - need at least products an one trader
    if (argc < 3) {
        printf("Not enough arguments\n");
        return 1;
    }

    //init exchange
    exchange ex;
    ex.no_of_orders = 0;
    ex.order_head = NULL;

    //get products out
    read_products(argv[1], &ex);

    //construct traders
    ex.no_of_traders = argc - 2; //discount the filename and products file.
    construct_traders(argv, &ex);

    struct order o;
    char buffer[] = "BUY 1 GPU 10 500;";
    parse_trader_order(buffer, &ex, &o, 0);
    add_order(&ex, o);

    char buffer2[] = "BUY 2 GPU 5 400;";
    parse_trader_order(buffer2, &ex, &o, 0);
    add_order(&ex, o);


    char buffer4[] = "SELL 3 GPU 2 250;";
    parse_trader_order(buffer4, &ex, &o, 1);
    add_order(&ex, o);

    char buffer5[] = "SELL 4 GPU 6 150;";
    parse_trader_order(buffer5, &ex, &o, 1);
    add_order(&ex, o);

    char buffer6[] = "SELL 5 GPU 8 350;";
    parse_trader_order(buffer6, &ex, &o, 1);
    add_order(&ex, o);

    // o.order_id = 15;
    // o.command = BUY;
    // add_order(&ex, o);

    // struct order o;

    // char buffer[] = "BUY 1 GPU 10 500;";
    // parse_trader_order(buffer, ex, &o, 0);
    // add_order(ex, o);

    // char buffer7[] = "SELL 5 GPU 8 500;";
    // parse_trader_order(buffer7, ex, &o, 1);
    // add_order(ex, o);

    // char buffer2[] = "BUY 2 GPU 5 400;";
    // parse_trader_order(buffer2, ex, &o, 0);
    // add_order(ex, o);

    // char buffer6[] = "SELL 5 GPU 8 350;";
    // parse_trader_order(buffer6, ex, &o, 1);
    // add_order(ex, o);

    // char buffer8[] = "BUY 3 GPU 2 250;";
    // parse_trader_order(buffer8, ex, &o, 1);
    // add_order(ex, o);


    // char buffer4[] = "BUY 3 GPU 2 250;";
    // parse_trader_order(buffer4, ex, &o, 1);
    // add_order(ex, o);

    // char buffer5[] = "SELL 4 GPU 6 150;";
    // parse_trader_order(buffer5, ex, &o, 1);
    // add_order(ex, o);

    

    // char buffer9[] = "BUY 4 GPU 6 150;";
    // parse_trader_order(buffer9, ex, &o, 1);
    // add_order(ex, o);
    


    // print_orders(&ex);

    // int i = 0;
    // while (i < ex.no_of_products) {
    //     printf("%s: BUY - %d SELL - %d\n", ex.products[i].name, ex.products[i].num_orders[BUY],  ex.products[i].num_orders[SELL]);
    //     i ++;
    // }
    // cancel_order(&ex, 230, 0);
    
    // amend_order(&ex, 25, 0, 999, 999);

    // print_orders(&ex);
    
    // int i = 0;
    // while (i < ex.no_of_products) {
    //     printf("%s: BUY - %d SELL - %d\n", ex.products[i].name, ex.products[i].num_orders[BUY],  ex.products[i].num_orders[SELL]);
    //     printf("is there at least one buy and sell? %d\n\n", check_for_BUY_SELL(&(ex.products[i])));
    //     print_orders_product(&(ex.products[i]), BUY);
    //     print_orders_product(&(ex.products[i]), SELL);

        
    //     i ++;
    // }

    while (check_for_BUY_SELL(&(ex.products[0]))) {
        if (!match_order(&ex, &(ex.products[0]))) {
            break;
        }
        printf("\n");
    }
    // match_order(&ex, &(ex.products[0])); //should be 1 with 4
    // match_order(&ex, &(ex.products[0])); //should be 1 wth 3
    // match_order(&ex, &(ex.products[0])); //should be 1 with 5
    // match_order(&ex, &(ex.products[0])); //should be 1 with 5

    // int i = 0;
    // while (i < ex.no_of_products) {
    //     printf("%s: BUY - %d SELL - %d\n", ex.products[i].name, ex.products[i].num_orders[BUY],  ex.products[i].num_orders[SELL]);
    //     printf("is there at least one buy and sell? %d\n\n", check_for_BUY_SELL(&(ex.products[i])));
    //     print_orders_product(&(ex.products[i]), BUY);
    //     print_orders_product(&(ex.products[i]), SELL);

        
    //     i ++;
    // }

    print_traders(&ex);

    free_orders(&ex);

        // print_orders(&ex);
    

    //free everything
    free(ex.products);
    free(ex.traders);
}

