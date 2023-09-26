#include "pe_common.h"
#include "orders.h"
#include "products.h"
#include "trader.h"

volatile sig_atomic_t something_to_read = 0;
volatile sig_atomic_t child_disconnected = -1;

void sigusr_function(int signo, siginfo_t* sinfo, void* context) {
        something_to_read = sinfo->si_pid;
}
void sigchild_function(int signo, siginfo_t* sinfo, void* context) {
        child_disconnected = sinfo->si_pid;
}



int main(int argc, char ** argv) {
    //check for enough arguments - need at least products an one trader
    if (argc < 3) {
        printf("Not enough arguments\n");
        return 1;
    }
    

    //set up handler
    struct sigaction sigusr1_handler;
    sigusr1_handler.sa_sigaction = sigusr_function;
    sigusr1_handler.sa_flags = SA_SIGINFO;

    if (sigaction(SIGUSR1, &sigusr1_handler, NULL) == -1) {
        perror("sigaction failed");
        return 1;
    }

    struct sigaction sigchild_handler;
    sigchild_handler.sa_sigaction = sigchild_function;
    sigchild_handler.sa_flags = SA_SIGINFO;

    if (sigaction(SIGCHLD, &sigchild_handler, NULL) == -1) {
        perror("sigaction failed");
        return 1;
    }


    printf("[PEX] Starting\n");

    //create exchange
    exchange * ex = malloc(sizeof(exchange));
    ex->balance = 0;

    if (ex == NULL) {
            perror("malloc was unsuccessful");
        }

    //GET PRODUCTS
    read_products(argv[1], ex);
    pex_print_products(ex);
    

    ex->no_of_traders = argc - 2;
    ex->traders = malloc((argc - 2) * sizeof(trader));
    ex->order_head = NULL;

    int i = 0;
    while (i < ex->no_of_traders) {
        ex->traders[i].id = i;
        ex->traders[i].pid = -1;
        ex->traders[i].active = 1;
        ex->traders[i].next_id = 0;
        ex->traders[i].balances = malloc(sizeof(long) * ex->no_of_products);
        ex->traders[i].executable_name = my_strdup(argv[i+2]);
        ex->traders[i].positions = malloc(sizeof(int) * ex->no_of_products);

        if (ex->traders[i].balances == NULL || ex->traders[i].positions == NULL) {
            perror("malloc was unsuccessful");
        }

        int p = 0;
        while (p < ex->no_of_products) {
            ex->traders[i].positions[p] = 0;
            ex->traders[i].balances[p] = 0;
            p ++;
        }

        i ++;
    }

    i = 0;
    while (i < ex->no_of_traders) {
        //ESTABLISH PIPES
        char pe_exchange[30];
        sprintf(pe_exchange, FIFO_EXCHANGE, i);
        unlink(pe_exchange);
        mkfifo(pe_exchange, 0666);
        printf("[PEX] Created FIFO %s\n", pe_exchange);

        char pe_trader[30];
        sprintf(pe_trader, FIFO_TRADER, i);
        unlink(pe_trader);
        mkfifo(pe_trader, 0666);
        printf("[PEX] Created FIFO %s\n", pe_trader);


        int child_pid = fork();

        if (child_pid == 0) {
        // if (1) {
            char id_no[16];
            sprintf(id_no, "%d", i);

            char* args[] = {ex->traders[i].executable_name, id_no, NULL};
            printf("[PEX] Starting trader %d (%s)\n", ex->traders[i].id, ex->traders[i].executable_name);
            execvp(args[0], args);
            perror("exec unsuccessful!");
        }

        else {
            ex->traders[i].pid = child_pid;

            //CONNECT THE PIPES
            ex->traders[i].e_to_t = open(pe_exchange, O_WRONLY);
            printf("[PEX] Connected to %s\n", pe_exchange);
            ex->traders[i].t_to_e = open(pe_trader, O_RDONLY);
            printf("[PEX] Connected to %s\n", pe_trader);

            // sleep(1);
        }
            

        i++;
    }
    

    //SEND OPEN MESSAGE
    i = 0;
    while (i < ex->no_of_traders) {
        send_message_to_trader(ex, i, MARKET_OPEN_MESSAGE);
        i ++;
    }

    int order_num = 0;

    //WAIT FOR REPLY
    while (1) {
        if (child_disconnected != -1) {
            int id = get_trader_id_from_pid(ex, child_disconnected);
            child_disconnected = -1;
            printf("[PEX] Trader %d disconnected\n", id);
            ex->traders[id].active = 0;
            if (check_for_active_traders(ex) == 0) {
                                break;
            }
        }

        if (something_to_read != 0) {
            int pid = something_to_read;
            something_to_read = 0;

                        int id = get_trader_id_from_pid(ex, pid);


            char buffer[50];
            get_message(ex->traders[id].t_to_e, buffer);
                        struct order o;
            int success = parse_trader_order(buffer, ex, &o, id, order_num);
            if (o.command == INVALID || success == 0) {
                char reply[50];
                sprintf(reply, "INVALID;");
                send_message_to_trader(ex, id, reply);
                continue;
            }

            else if (o.command == BUY || o.command == SELL){
                char reply[50];
                sprintf(reply, "ACCEPTED %d;", o.order_id);
                send_message_to_trader(ex, id, reply);
                order_num ++;
                add_order(ex, o);
                
            }

            else if (o.command == CANCEL) {
                // struct order deleted_order;
                cancel_order(ex, o.order_id, id, &o);
                char reply[50];
                sprintf(reply, "CANCELLED %d;", o.order_id);
                send_message_to_trader(ex, id, reply);

                o.quantity = 0;
                o.price = 0;
            }

            else if (o.command == AMEND) {
                // amend_order(ex, o.order_id, id, o.price, o.quantity);
                struct order old_order;
                cancel_order(ex, o.order_id, id, &old_order);

                old_order.price = o.price;
                old_order.quantity = o.quantity;
                add_order(ex, old_order);
                
                char reply[50];
                sprintf(reply, "AMENDED %d;", o.order_id);
                send_message_to_trader(ex, id, reply);

                o.command = old_order.command;
            }

            char broadcast[50];
            if (o.command == BUY) {
                sprintf(broadcast, "MARKET BUY %s %d %d;", o.order_product->name, o.quantity, o.price);
            } else if (o.command == SELL) {
                sprintf(broadcast, "MARKET SELL %s %d %d;", o.order_product->name, o.quantity, o.price);
            }

            int t = 0;
            while (t < ex->no_of_traders) {
                if (t == id) {
                    t++;
                    continue;
                }
                send_message_to_trader(ex, t, broadcast);
                t ++;
            }

            
            while (check_for_BUY_SELL(o.order_product)) {
                if (!match_order(ex, o.order_product->product_id, &o)) {
                    break;
                }
            
            }

            
            report_exchange(ex);

            // if (o.price == 402) {
            //     break;
            // }

            if (something_to_read == 0 && child_disconnected == -1) {
                pause();
            }
            
        }
    }

    // print_orders(ex);

    // int status;
    // wait(&status);


    i = 0;
    while (i < ex->no_of_traders) {
        close(ex->traders[i].e_to_t);
        close(ex->traders[i].t_to_e);
                char pe_exchange[30];
        sprintf(pe_exchange, FIFO_EXCHANGE, i);
        unlink(pe_exchange);

        char pe_trader[30];
        sprintf(pe_trader, FIFO_TRADER, i);
        unlink(pe_trader);
        i ++;
    }

    printf("[PEX] Trading completed\n");
    printf("[PEX] Exchange fees collected: $%ld\n", ex->balance);
  
    free_orders(ex);

    i = 0;
    while (i < ex->no_of_traders) {
        free(ex->traders[i].executable_name);
        free(ex->traders[i].positions);
        free(ex->traders[i].balances);
        i ++;
    }

    

    free(ex->products);
    free(ex->traders);
    free(ex);

}