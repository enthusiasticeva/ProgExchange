#include "pe_common.h" 
#include "products.h"
#include "trader.h"


// function inspired by https://stackoverflow.com/a/14770895
char* my_strdup(char* c) { 
    char* new_location = malloc(strlen(c) + 1);
    if (new_location == NULL) {
        perror("malloc was unsuccessful");
    }

    if (new_location != NULL) {
        strcpy(new_location, c);
    }

    return new_location;
}

int check_alphanumeric(char c) {
    if ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c == ' ')) {
        return 1;
    }
    return 0;
}


int parse_trader_order(char* buffer, exchange* ex ,struct order * my_order, int trader_id, int order_num) {
    printf("[PEX] [T%d] Parsing command: <%s>\n", trader_id, buffer);
    my_order->order_trader = &(ex->traders[trader_id]);
    my_order->next_order = NULL;
    my_order->next_similar_order = NULL;
    my_order->next_combined_order = NULL;
    my_order->order_num = order_num;

    char * token = strtok(buffer, " ");
    if (token == NULL) {
        my_order -> command = INVALID;
        return 0;
    }

    //get command
    if (strcmp(token, "BUY") == 0) {
        my_order -> command = BUY;
    } else if (strcmp(token, "SELL") == 0) {
        my_order -> command = SELL;
    } else if (strcmp(token, "AMEND") == 0) {
        my_order -> command = AMEND;
    } else if (strcmp(token, "CANCEL") == 0) {
        my_order -> command = CANCEL;
    } else {
        my_order -> command = INVALID;
    }

    //extract rest of info depending on command 
    //note atoi ignores non digit characters i.e. the ; on the end.
    if (my_order -> command == BUY || my_order -> command == SELL) {

        token = strtok(NULL, " ");
        if (token == NULL) {
            my_order -> command = INVALID;
            return 0;
        }
        my_order -> order_id = atoi(token);
        
        if (my_order->order_id < 0 || my_order->order_id != my_order->order_trader->next_id) {
            my_order -> command = INVALID;
            return 0;
        }

        my_order->order_trader->next_id += 1;
        
        token = strtok(NULL, " ");
        if (token == NULL) {
            my_order -> command = INVALID;
            return 0;
        }
        my_order -> order_product = find_matching_product(token, ex);
        if (my_order -> order_product == NULL) {
            my_order -> command = INVALID;
            return 0;
        }
        
        token = strtok(NULL, " ");
        if (token == NULL) {
            my_order -> command = INVALID;
            return 0;
        }
        my_order -> quantity = atoi(token);
        

        token = strtok(NULL, " ");
        if (token == NULL) {
            my_order -> command = INVALID;
            return 0;
        }
        my_order -> price = atoi(token);
                
    }

    else if (my_order -> command == AMEND) {
        token = strtok(NULL, " ");
        if (token == NULL) {
            my_order -> command = INVALID;
            return 0;
        }
        my_order -> quantity = atoi(token);

        token = strtok(NULL, " ");
        if (token == NULL) {
            my_order -> command = INVALID;
            return 0;
        }
        my_order -> price = atoi(token);
    }

    else if (my_order->command == CANCEL) {
        token = strtok(NULL, " ");
        // printf("'%s'\n", token);
        if (token == NULL) {
            my_order -> command = INVALID;
            return 0;
        }
        my_order -> order_id = atoi(token);
        
        if (my_order->order_id < 0 || my_order->order_id >= my_order->order_trader->next_id) {
            my_order -> command = INVALID;
            return 0;
        }
    }

    if (my_order->quantity < 1 || my_order->quantity > 999999 || my_order->price < 1 || my_order->price > 999999) {
        my_order -> command = INVALID;
        return 0;
    }
    
    return 1;
}

void add_order(exchange * ex, struct order order) {

    struct order* temp = malloc(sizeof(order));
    if (temp == NULL) {
        perror("malloc was unsuccessufl");
    }

    *temp = order;
    temp->next_similar_order = NULL;
    temp->next_combined_order = NULL;
    struct order* current = ex->order_head;

    //ADD TO MAIN LINKED LIST
    //if there are currently no orders, make the head
    if (current == NULL) {
        ex->order_head = temp;
        // *(ex->order_head) = order;
    }
    //if there are other orders, add to the end
    else {
        
        while (current->next_order != NULL) {
            current = current -> next_order;
        }
        
        current->next_order = temp;
        // *(current->next_order) = order;
        
    }

    //ADD TO PRODUCT BUY/SELL LINKED LIST
    product * current_product = order.order_product;
    
    //if there are currently no orders, make the head
    if (current_product->num_orders[order.command] == 0) {

        current_product->buy_sell_heads[order.command] = temp;
        
    }

    //if the order to insert is less than/greater than the head...
    else {
        struct order* current = current_product->buy_sell_heads[order.command];
        if ((current->price < order.price && current->command == BUY)||
            (current->price > order.price && current->command == SELL)) {
                        
            //add the node as the head
            current_product->buy_sell_heads[order.command] = temp;
            // order.next_similar_order = current_product->buy_sell_heads[order.command];

            //set it's next
            current_product->buy_sell_heads[order.command]->next_similar_order = current;
        }

        else if (current_product->num_orders[order.command] == 1){
                        current->next_similar_order = temp;
            temp->next_similar_order = NULL;
        }

        else {
            
            while (current->next_similar_order != NULL) { //BUY is in descending order
                if (current->next_similar_order->price < order.price && order.command == BUY) {
                    break;
                }
                if (current->next_similar_order->price > order.price && order.command == SELL) {
                    break;
                }
                current = current -> next_similar_order;
                
            } 


            struct order* after = current->next_similar_order;
            current->next_similar_order = temp;
            temp->next_similar_order = after;


        }

    }

    //ADD TO PRODUCT COMBINED SORTED ORDERS (DESCENDING)
    if (current_product->combined_orders == NULL) {

        current_product->combined_orders = temp;
        
    }

    //if the order to insert is less than/greater than the head...
    else {
        struct order* current = current_product->combined_orders;
        if (current->price <= order.price){
            
            //add the node as the head
            current_product->combined_orders = temp;
            // order.next_similar_order = current_product->buy_sell_heads[order.command];

            //set it's next
            current_product->combined_orders->next_combined_order = current;
        }

        else if (current->next_combined_order == NULL){
                        current->next_combined_order = temp;
            temp->next_combined_order = NULL;
        }

        else {
            
            while (current->next_combined_order != NULL) { //BUY is in descending order
                if (current->next_combined_order->price <= order.price) {
                    break;
                }
                current = current -> next_combined_order;
                
            } 


            struct order* after = current->next_combined_order;
            current->next_combined_order = temp;
            temp->next_combined_order = after;
        }

    }

    // print_order(temp);

    //increment the order counters
    ex->no_of_orders += 1;
    order.order_product->num_orders[order.command] += 1;

}

int cancel_order(exchange * ex, int order_id, int trader_id, struct order* deleted_order) {
    struct order* current = ex->order_head;
    struct order* prev = NULL;
    struct order* to_delete = NULL;

    //REMOVE FROM MAIN LINKED LIST
    while (current != NULL) {
        if (current->order_id == order_id && current->order_trader->id == trader_id) {
            
            if (prev == NULL) {
                ex->order_head = current->next_order; 
            }

            else {
                prev->next_order = current->next_order;
            }
            

            ex->no_of_orders -= 1;

            to_delete = current;
            
            // free(current);
            break;
        }
        prev = current;
        current = current->next_order;
    }
    
    if (deleted_order != NULL) {
        *deleted_order = *current;
    }
    
    

    //REMOVE FROM PRODUCT LINKED LIST
    product * current_product = to_delete->order_product;
    current = current_product->buy_sell_heads[to_delete->command];
    
    prev = NULL;
    while (current != NULL) {
        if (current->order_id == order_id && current->order_trader->id == trader_id) {
            if (prev == NULL) {
                current_product->buy_sell_heads[to_delete->command] = current->next_similar_order;
            }

            else {
                prev -> next_similar_order = current->next_similar_order;
            }


            current_product->num_orders[current->command] -= 1;
            break;
        }
        prev = current;
        current = current->next_similar_order;
    }

    //REMOVE FROM PRODUCT LINKED LIST
    current = current_product->combined_orders;
    
    prev = NULL;
    while (current != NULL) {
        if (current->order_id == order_id && current->order_trader->id == trader_id) {
            if (prev == NULL) {
                current_product->combined_orders = current->next_combined_order;
            }

            else {
                prev -> next_combined_order = current->next_combined_order;
            }

            break;
        }
        prev = current;
        current = current->next_combined_order;
    }


    free(to_delete);
    return 0;
}

int amend_order(exchange * ex, int order_id, int trader_id, int price, int quantity) {
    struct order* current = ex->order_head;

    while (current != NULL) {
        if (current->order_id == order_id && current->order_trader->id == trader_id) {
            current->price = price;
            current->quantity = quantity;
            return 1;
        }
        current = current->next_order;
    }
    return 0;
}


void free_orders(exchange * ex) {
    struct order* current = ex->order_head;

    while (current != NULL) {
        struct order* next = current -> next_order;
        free(current);
        current = next;
    }
    ex->order_head = NULL;
    ex->no_of_orders = 0;

    int i = 0;
    while (i  < ex->no_of_products) {
        int x = 1;
        while (x < 3) {
            ex->products[i].buy_sell_heads[x] = NULL;
            x ++;
        }
        ex->products[i].combined_orders = NULL;
        i ++;
    }
}

int check_for_BUY_SELL(product *product) {
    return (product->num_orders[BUY]>0 && product->num_orders[SELL]>0);
}



void fill_order(struct order* buyer_order, struct order* seller_order, exchange* ex, int price, int product_id, int quantity, enum command fee_payer) {
    trader* buyer = buyer_order->order_trader;
    trader* seller = seller_order->order_trader;

    

    //transfer money
    long total_price = (long)price * (long) quantity;
    long fee = round(0.01 * total_price);

    if (buyer_order->order_num < seller_order->order_num) {//implies BUYER is the older order
        printf("[PEX] Match: Order %d [T%d], New Order %d [T%d], value: $%ld, fee: $%ld.\n", buyer_order->order_id, buyer->id,seller_order->order_id, seller->id,  total_price, fee);
    } else {
        printf("[PEX] Match: Order %d [T%d], New Order %d [T%d], value: $%ld, fee: $%ld.\n", seller_order->order_id, seller->id,buyer_order->order_id, buyer->id,  total_price, fee);
    }

    buyer->balances[product_id] -= (total_price);
    seller->balances[product_id] += total_price;

    if (fee_payer == BUY) {
        buyer->balances[product_id] -= fee;
    } else {
        seller->balances[product_id] -= fee;
    }

    ex->balance += fee;

    //transfer quantity
    buyer->positions[product_id] += quantity;
    seller->positions[product_id] -= quantity;

    char reply[50];
    sprintf(reply, "FILL %d %d;", buyer_order->order_id, quantity);
    send_message_to_trader(ex, buyer->id, reply);

    sprintf(reply, "FILL %d %d;", seller_order->order_id, quantity);
    send_message_to_trader(ex, seller->id, reply);
}

int match_order(exchange *ex, int product_id, struct order* latest_order) {
    // print_order(latest_order);
    product* product = &(ex->products[product_id]);
    
    struct order* buy_current = product->buy_sell_heads[BUY];
    struct order* sell_current = product->buy_sell_heads[SELL];

    if (buy_current == NULL || sell_current == NULL || product->num_orders[BUY] == 0 || product->num_orders[SELL] == 0) {
                return 0;
    }

        
    
    if (buy_current->price < sell_current->price) {
                return 0;
    }
    int price;
    price = 0;
    enum command fee_payer;
    

    //ADD FEE PAYERS
    if (buy_current->order_num < sell_current->order_num) {
        price = buy_current->price;
        fee_payer = SELL;
    } else {
        price = sell_current->price;
        fee_payer = BUY;
    }
    

    if (buy_current->quantity > sell_current->quantity) {
        

        fill_order(buy_current, sell_current, ex, price, product->product_id, sell_current->quantity, fee_payer);

        buy_current->quantity -= sell_current->quantity;
        sell_current->quantity = 0;

        cancel_order(ex, sell_current->order_id, sell_current->order_trader->id, NULL);
        
    }
    else if (buy_current->quantity < sell_current->quantity) {
        
        fill_order(buy_current, sell_current, ex, price, product->product_id, buy_current->quantity, fee_payer);


        sell_current->quantity -= buy_current->quantity;
        buy_current->quantity = 0;
        cancel_order(ex, buy_current->order_id, buy_current->order_trader->id, NULL);

        
    } else {
                fill_order(buy_current, sell_current, ex, price, product->product_id, buy_current->quantity, fee_payer);


        sell_current->quantity = 0;
        buy_current->quantity = 0;
        cancel_order(ex, buy_current->order_id, buy_current->order_trader->id, NULL);
        cancel_order(ex, sell_current->order_id, sell_current->order_trader->id, NULL);

        
    }
    

        // print_trader(ex, sell_current->order_trader);
    // print_trader(ex, buy_current->order_trader);


    return 1;
}



int count_levels(product* product, enum command command) {
    struct order* current = product->buy_sell_heads[command];

    int current_price = -1;

    int levels = 0;

    while (current != NULL) {
        if (current->price !=current_price) {
            levels += 1;
            current_price = current->price;
        }


        current = current->next_similar_order;
    }

    return levels;
}

// int find_nth_biggest_price(product* product, int n, enum command* command) {
//     struct order* buy_current = product->buy_sell_heads[BUY];
//     struct order* sell_current = product->buy_sell_heads[SELL];

//     int i = 0;

//     if (buy_current->price > sell_current->price) {
//         int max_price = buy_current->price;
//     } else {
//         int max_price = sell_current->price;
//     }

//     while (i < n) {
//         i ++;
//     }
// }

void print_orders(exchange* ex) {
    int p = 0;
    while (p < ex->no_of_products) {
        product * current_product = &(ex->products[p]);
        int buy_levels = count_levels(current_product, BUY);
        int sell_levels = count_levels(current_product, SELL);

        printf("[PEX]\tProduct: %s; Buy levels: %d; Sell levels: %d\n", current_product->name, buy_levels, sell_levels);
        p ++;

        struct order* current = current_product->combined_orders;
        if (current == NULL) {
            continue;
        }

        enum command current_command = current->command;
        int current_price = current->price;

        int order_count = 0;
        int order_quantity = 0;

        if (current->next_combined_order == NULL) {
            order_quantity += current->quantity;
            if (current_command == BUY) {
            printf("[PEX]\t\tBUY %d @ $%d (1 order)\n", order_quantity, current_price);
            }
            else if (current_command == SELL) {
                printf("[PEX]\t\tSELL %d @ $%d (1 order)\n", order_quantity, current_price);
            }
            continue;
        }

        while (current != NULL) {
            if (current->price == current_price && current->command == current_command) {
                order_count += 1;
                order_quantity += current->quantity;
            } else {
                if (current_command == BUY) {
                    printf("[PEX]\t\tBUY %d @ $%d", order_quantity, current_price);
                }
                else if (current_command == SELL) {
                    printf("[PEX]\t\tSELL %d @ $%d", order_quantity, current_price);
                }

                if (order_count > 1) {
                    printf(" (%d orders)\n", order_count);
                } else {
                    printf(" (%d order)\n", order_count);
                }

                current_command = current->command;
                current_price = current->price;
                order_quantity = current->quantity;
                order_count = 1;
                
            }
            
            current = current->next_combined_order;
        }
        if (current_command == BUY) {
            printf("[PEX]\t\tBUY %d @ $%d", order_quantity, current_price);
        }
        else if (current_command == SELL) {
            printf("[PEX]\t\tSELL %d @ $%d", order_quantity, current_price);
        }
        if (order_count > 1) {
            printf(" (%d orders)\n", order_count);
        } else {
            printf(" (%d order)\n", order_count);
        }

    }
}

void report_exchange(exchange * ex) {
    printf("[PEX]\t--ORDERBOOK--\n");
    print_orders(ex);
    printf("[PEX]\t--POSITIONS--\n");
    print_traders(ex);
}

int check_for_active_traders(exchange* ex) {
    int active = 0;
    int t = 0;
    while (t < ex->no_of_traders) {
        active = active || ex->traders[t].active;
        t ++;
    }
    return active;
}