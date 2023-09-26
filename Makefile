CC=gcc
CFLAGS=-Wall -Werror -Wvla -O0 -std=c11 -g #-fsanitize=address,leak
LDFLAGS=-lm
BINARIES= pe_exchange pe_trader

all: $(BINARIES)

.PHONY: clean
clean:
	rm -f $(BINARIES) orders

pe_exchange: 
	$(CC) $(CFLAGS) $(LDFLAGS) pe_exchange.c -o pe_exchange orders.c trader.c products.c

pe_trader: 
	$(CC) $(CFLAGS) $(LDFLAGS) pe_trader.c -o pe_trader

test:  
	$(CC) -o test $(FLAGS) tests/unit_tests.c orders.c trader.c products.c tests/libcmocka-static.a