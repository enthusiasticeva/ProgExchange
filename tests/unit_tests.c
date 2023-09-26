#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <setjmp.h>
#include "cmocka.h"

#include "../pe_common.h"
#include "../orders.h"
#include "../products.h"
#include "../trader.h"


/* TESTS */
static void null_test_success(void **state) {
    (void) state; /* unused */
}

static void test_add_buy_sell(void **state) {
    //SET UP
    exchange * ex = malloc(sizeof(exchange));

    //GET PRODUCTS
    read_products("products.txt", ex);
    ex->no_of_traders = 2;
    ex->traders = malloc(2*sizeof(trader));
    
    int i = 0;
    while (i < ex->no_of_traders) {
        ex->traders[i].id = i;
        ex->traders[i].pid = -1;
        ex->traders[i].active = 1;
        ex->traders[i].next_id = 0;
        ex->traders[i].balances = malloc(sizeof(int) * ex->no_of_products);
        ex->traders[i].positions = malloc(sizeof(int) * ex->no_of_products);

        int p = 0;
        while (p < ex->no_of_products) {
            ex->traders[i].positions[p] = 0;
            ex->traders[i].balances[p] = 0;
            p ++;
        }

        i ++;
    }

    //ACCTUAL TESTING
    assert_int_equal(ex->no_of_orders, 0);
    i = 0;
    while (i < ex->no_of_products) {
        assert_null(ex->products[i].combined_orders);
        assert_null(ex->products[i].buy_sell_heads[BUY]);
        assert_null(ex->products[i].buy_sell_heads[SELL]);
        i ++;
    }

    struct order o;
    int success = parse_trader_order("BUY 0 GPU 30 500;", ex, &o, 0, 0);
    assert_int_equal(success, 1);
    add_order(ex, o);

    assert_int_equal(ex->no_of_orders, 1);
    assert_non_null(ex->products[0].buy_sell_heads[BUY]);
    assert_non_null(ex->products[0].combined_orders);
    assert_int_equal(ex->products[0].num_orders[BUY], 1);
    assert_int_equal(ex->products[0].num_orders[SELL], 0);

    success = parse_trader_order("SELL 1 Router 20 600;", ex, &o, 0, 1);
    assert_int_equal(success, 1);
    add_order(ex, o);

    success = parse_trader_order("SELL 0 Router 15 500;", ex, &o, 1, 2);
    assert_int_equal(success, 1);
    add_order(ex, o);

    assert_non_null(ex->products[1].buy_sell_heads[SELL]);
    assert_non_null(ex->products[1].buy_sell_heads[SELL]->next_similar_order);
    assert_non_null(ex->products[1].combined_orders);
    assert_non_null(ex->products[1].combined_orders->next_similar_order);
    assert_int_equal(ex->products[1].num_orders[BUY], 0);
    assert_int_equal(ex->products[1].num_orders[SELL], 2);

    assert_int_equal(ex->no_of_orders, 3);


    //PACK DOWN
    i = 0;
    while (i < ex->no_of_traders) {
        free(ex->traders[i].positions);
        free(ex->traders[i].balances);
        i ++;
    }

    free_orders(ex);

    free(ex->traders);
    free(ex->products);
    free(ex);
}

static void test_amend(void **state) {

}

static void test_cancel(void **state) {

}

static void test_parse_wrong_orders (void **state) {
    //incorrect (numbers too big)
    //incorrect (numbers too small)
    //incorrect (arguments not given)
    //incorrect order id wrong
}

static void test_parse_correct_orders (void **state) {
    //incorrect (numbers too big)
    //incorrect (numbers too small)
    //incorrect (arguments not given)
    //incorrect order id wrong
}

// static void stack_initialised(void **state) {
//     stack* s = stack_alloc();
//     assert_non_null(s);
//     stack_free(s);
// }

// static void stack_push(void **state) {
//     stack* s = stack_alloc();

//     push(s, 100);
//     assert_int_equal(peek(s).val, 100);
//     assert_int_equal(peek(s).failed, 0);


//     push(s, 200);
//     push(s,300);

//     assert_int_equal(peek(s).val, 300);
//     assert_int_equal(peek(s).failed, 0);

//     stack_free(s);
    
// }

// static void stack_pop(void **state) {
//     stack* s = stack_alloc();

//     s_result res = pop(s);
//     assert_int_equal(res.failed, 1);

//     push(s, 100);
//     push(s, 200);
//     push(s, 300);

//     res = pop(s);
//     assert_int_equal(res.val, 300);
//     assert_int_equal(res.failed, 0);

//     assert_int_equal(peek(s).val, 200);
//     assert_int_equal(s->size, 2);

//     stack_free(s);
    
// }

/*RUNNING TESTS*/
int main(void) {
    const struct CMUnitTest tests[] = {
        // cmocka_unit_test(null_test_success),
        // cmocka_unit_test(stack_initialised),
        // cmocka_unit_test(stack_push),
        // cmocka_unit_test(stack_pop)
        //cmocka_unit_test(),
    };
 
    return cmocka_run_group_tests(tests, NULL, NULL);
}