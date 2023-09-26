#include "pe_trader.h"
// #include "pe_common.h"


enum trader_command {INVALID, BUY, SELL, AMEND, CANCEL, FILL, ACCEPTED};

struct trader_order {
    enum trader_command command;
    int order_id;
    char * product;
    int quantity;
    int price;
};

int atoi(const char s[]);

void get_message(int file_to_read, char * buffer);

void parse_order(char * buffer, struct trader_order * my_order);

void build_order(char * dest, struct trader_order * my_order);

void print_order(struct trader_order * my_order);


volatile int something_to_read = 0;

void handler(int signo, siginfo_t* sinfo, void* context) {
    // printf ("trader: received SIGUSR1\n");
    something_to_read = 1;
}


int main(int argc, char ** argv) {
    if (argc < 2) {
        printf("Not enough arguments\n");
        return 1;
    }

    // register signal handler
    // signal(SIGUSR1, handler);
    struct sigaction sig_handler;
    sig_handler.sa_sigaction = handler;
    sig_handler.sa_flags = SA_SIGINFO;

    if (sigaction(SIGUSR1, &sig_handler, NULL) == -1) {
        perror("sigaction failed");
        return 1;
    }

    // connect to named pipes
    //open pipe from exchange to trader as readonly
    char pe_exchange[30];// = "/tmp/pe_exchange_0";
    sprintf(pe_exchange, FIFO_EXCHANGE, atoi(argv[1]));
    int e_to_t = open(pe_exchange, O_RDONLY);


    //open pipe from trader to exchange in writeonly
    char pe_trader[30];// = "/tmp/pe_trader_0";
    sprintf(pe_trader, FIFO_TRADER,atoi( argv[1]));
    int t_to_e = open(pe_trader, O_WRONLY);

    // int pid;
    // printf("pid: ");
    // scanf("%d", &pid);

    //GET MARKET OPEN;
    char buffer[MAX_COMMAND_LENGTH];

    // printf("trader: watiing for signal\n");
    while (1) {
        if (something_to_read == 1) {
            something_to_read = 0;
            get_message(e_to_t, buffer);

            if (strcmp(buffer, "MARKET OPEN;") == 0) {
                break;
            }
        }

    }
    // printf("trader: market is open!\n");

    

    int next_order_id = 0;
    
    // event loop:
    while (1) {
        // wait for exchange update (MARKET message)
        if (something_to_read) {
            something_to_read = 0;

            // printf("getting in here!\n");
            
            get_message(e_to_t, buffer);
            // printf("Order receieved: %s\n", buffer);


            //parse order

            struct trader_order received_order;
            parse_order(buffer, &received_order);
            // print_order(&received_order);

            if (received_order.command == FILL) {
                continue;
            }

            if (received_order.command == SELL && received_order.quantity >= 1000) {
                // printf("breaking!");
                break;
            }

            if (received_order.command == SELL) {
                struct trader_order my_order;
                my_order.command = BUY;
                my_order.order_id = next_order_id;
                next_order_id ++;
                my_order.product = received_order.product;
                my_order.quantity = received_order.quantity;
                my_order.price = received_order.price;

                char string_order[MAX_COMMAND_LENGTH];
                build_order(string_order, &my_order);
                // printf("Constructed order: %s\n", string_order);

                write(t_to_e, string_order, strlen(string_order));
                // write(t_to_e, "BUY 0 GPU 50 3499;", 18);
                kill(getppid(), SIGUSR1);
                // kill((pid_t)pid, SIGUSR1);

                //wait for accepted
                struct trader_order reply;

                while (1) {
                    if (something_to_read) {
                        something_to_read = 0;
                        get_message(e_to_t, buffer);
                        parse_order(buffer, &reply);

                        if (reply.command == ACCEPTED && reply.order_id == my_order.order_id) {
                            break;
                            
                        }
                    }
                }

                
                
            }

            

        }
        if (something_to_read == 0) {
            pause();
        }
    }
    


    
    // send order
    // wait for exchange confirmation (ACCEPTED message)
    close(t_to_e);
    close (e_to_t);
    return 0;
}


void get_message(int file_to_read, char * buffer) {
    int i = 0;
    char temp = 'a';
    // printf("loop is starting");
    // printf("\nREADING MESSAGE: ");
    while (i < MAX_COMMAND_LENGTH && temp != ';') {
        int success = read(file_to_read, &temp, 1);
         if (success < 1) {
            perror("reading failed");
        }
        // printf("%c", temp);
        
        buffer[i] = temp;
        
        i ++;
    }

    
    buffer[i] = '\0';
    // printf("loop is finishing: %s\n" , buffer);
}

int atoi(const char s[]) { 
    //will store our result
    int full_num = 0;

    // will go from the last character back
    int index = strlen(s)-1;

    // keeps track of our place value
    int place_val = 1;


    while (index >= 0) {

        if (s[index] < '0' || s[index] > '9') {
            index --;
            continue;
        }

        // get the digit as a number, times by the place value
        int num = (s[index]-'0') * place_val;

        // add to the result
        full_num = full_num + num;

        // increase the place value and decrease the index
        place_val *= 10;
        index --;
    }

    return full_num;
}

void parse_order(char * buffer, struct trader_order * my_order) {
    

    char * token = strtok(buffer, " ");
    
    //take the market off the front
    if (strcmp(token, "MARKET") == 0) {
        token = strtok(NULL, " ");
    }
    if (strcmp(token, "ACCEPTED") == 0) {
        my_order -> command = ACCEPTED;
        token = strtok(NULL, " ");
        my_order -> order_id = atoi(token);
        return;
    }
    if (strcmp(token, "FILL") == 0) {
        my_order -> command = FILL;
        token = strtok(NULL, " ");
        my_order -> order_id = atoi(token);

        token = strtok(NULL, " ");
        my_order -> quantity = atoi(token);
        return;
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
    }

    //extract rest of info depending on command 
    //note atoi ignores non digit characters i.e. the ; on the end.
    if (my_order -> command == BUY || my_order -> command == SELL) {
        token = strtok(NULL, " ");
        my_order -> product = token;

        token = strtok(NULL, " ");
        my_order -> quantity = atoi(token);

        token = strtok(NULL, " ");
        my_order -> price = atoi(token);
    }

    else if (my_order -> command == AMEND) {
        token = strtok(NULL, " ");
        my_order -> quantity = atoi(token);

        token = strtok(NULL, " ");
        my_order -> price = atoi(token);
    }
    return;
}


void build_order(char * dest, struct trader_order * my_order) {
    if (my_order -> command == BUY) {
        sprintf(dest, "BUY %d %s %d %d;", my_order -> order_id, my_order -> product, my_order -> quantity, my_order -> price);
    } 
    else if (my_order -> command == SELL) {
        sprintf(dest, "SELL %d %s %d %d;", my_order -> order_id, my_order -> product, my_order -> quantity, my_order -> price);
    } 
    else if (my_order -> command == AMEND) {
        sprintf(dest, "AMEND %d %d %d;", my_order -> order_id, my_order -> quantity, my_order -> price);
    } 
    else if (my_order -> command == CANCEL) {
        sprintf(dest, "CANCEL %d;", my_order -> order_id);
    } 

}

void print_order(struct trader_order * my_order) {
    printf("RECEIVED MESSAGE: ");
    if (my_order -> command == BUY) {
        printf("BUY %d %s %d %d;", my_order -> order_id, my_order -> product, my_order -> quantity, my_order -> price);
    } 
    else if (my_order -> command == SELL) {
        printf("SELL %d %s %d %d;", my_order -> order_id, my_order -> product, my_order -> quantity, my_order -> price);
    } 
    else if (my_order -> command == AMEND) {
        printf("AMEND %d %d %d;", my_order -> order_id, my_order -> quantity, my_order -> price);
    } 
    else if (my_order -> command == CANCEL) {
        printf("CANCEL %d;", my_order -> order_id);    
    }
    else {
        printf("none\n");
    }
    printf("\n");
}