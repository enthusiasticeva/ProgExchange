#include "pe_common.h"
#include "products.h"
#include "orders.h"

void send_message_to_trader(exchange* ex, int trader, char * message) {
        if (ex -> traders[trader].active == 0) {
        return;
    }

    int success = write(ex -> traders[trader].e_to_t, message, strlen(message));
    if (success < strlen(message)) {
            perror("writing to file failed");
    }
        kill(ex -> traders[trader].pid, SIGUSR1);
}

void send_message_to_exchange(int pipe, char * message) {
    
    int success = write(pipe, message, strlen(message));
    if (success < strlen(message)) {
            perror("writing to file failed");
    }
        kill(getppid(), SIGUSR1);
}

void get_message(int file_to_read, char * buffer) {
    int i = 0;
    char temp = 'a';
            while (i < MAX_COMMAND_LENGTH && temp != ';') {
        int success = read(file_to_read, &temp, 1);
        if (success < 1) {
            perror("reading failed");
        }
                
        buffer[i] = temp;
        
        i ++;
    }

    
    buffer[i-1] = '\0';
    }

int get_trader_id_from_pid(exchange* ex, int pid) {
    int i = 0;
    while (i < ex->no_of_traders) {
        if (ex->traders[i].pid == pid) {
            return ex->traders[i].id;
        }
        i ++;
    }
    return -1;
}

void print_trader(exchange* ex, trader* trader) {
    printf("[PEX]\tTrader %d: ", trader->id);

    int p = 0;
    while (p < ex->no_of_products) {
        printf("%s %d ($%ld)", ex->products[p].name, trader->positions[p], trader->balances[p]);
        if (p < ex->no_of_products - 1) {
            printf(", ");
        }
        p ++;
    }
    printf("\n");
}

void print_traders(exchange* ex) {
    int t = 0;

    while (t<ex->no_of_traders) {
        print_trader(ex, &(ex->traders[t]));
        t ++;
    }
}