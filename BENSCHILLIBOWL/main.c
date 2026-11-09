#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>

#include "BENSCHILLIBOWL.h"

// Feel free to play with these numbers! This is a great way to
// test your implementation.
#define BENSCHILLIBOWL_SIZE 100
#define NUM_CUSTOMERS 90
#define NUM_COOKS 10
#define ORDERS_PER_CUSTOMER 3
#define EXPECTED_NUM_ORDERS NUM_CUSTOMERS * ORDERS_PER_CUSTOMER

// Global variable for the restaurant.
BENSCHILLIBOWL *bcb;

/**
 * Thread funtion that represents a customer. A customer should:
 *  - allocate space (memory) for an order.
 *  - select a menu item.
 *  - populate the order with their menu item and their customer ID.
 *  - add their order to the restaurant.
 */
void* BENSCHILLIBOWLCustomer(void* tid) {
    int customer_id = (int)(long) tid;

    for (int i = 0; i < ORDERS_PER_CUSTOMER; i++) {
        // Allocate space (memory) for an order
        Order *order = (Order*) malloc(sizeof(Order));
        
        // Select a menu item & populate order details
        order->menu_item = PickRandomMenuItem();
        order->customer_id = customer_id;
        order->next = NULL;

        // Add order to the restaurant
        AddOrder(bcb, order);
    }
    return NULL;
}

/**
 * Thread function that represents a cook in the restaurant. A cook should:
 *  - get an order from the restaurant.
 *  - if the order is valid, it should fulfill the order, and then
 *    free the space taken by the order.
 * The cook should take orders from the restaurants until it does not
 * receive an order.
 */
void* BENSCHILLIBOWLCook(void* tid) {
    int cook_id = (int)(long) tid;
    int orders_fulfilled = 0;

    while (1) {
        Order *order = GetOrder(bcb);
        
        // Break out when no more orders are returned (GetOrder returns NULL)
        if (order == NULL) {
            break;
        }

        // Fulfill order and deallocate memory
        free(order);
        orders_fulfilled++;
    }

    printf("Cook #%d fulfilled %d orders\n", cook_id, orders_fulfilled);
    return NULL;
}

/**
 * Runs when the program begins executing. This program should:
 *  - open the restaurant
 *  - create customers and cooks
 *  - wait for all customers and cooks to be done
 *  - close the restaurant.
 */
int main() {
    // 1. Open the restaurant with size limit and expected total orders
    bcb = OpenRestaurant(BENSCHILLIBOWL_SIZE, EXPECTED_NUM_ORDERS);

    pthread_t customer_threads[NUM_CUSTOMERS];
    pthread_t cook_threads[NUM_COOKS];

    // 2. Create customer threads
    for (long i = 0; i < NUM_CUSTOMERS; i++) {
        pthread_create(&customer_threads[i], NULL, BENSCHILLIBOWLCustomer, (void*)i);
    }

    // Create cook threads
    for (long i = 0; i < NUM_COOKS; i++) {
        pthread_create(&cook_threads[i], NULL, BENSCHILLIBOWLCook, (void*)i);
    }

    // 3. Wait for all customer threads to finish placing orders
    for (int i = 0; i < NUM_CUSTOMERS; i++) {
        pthread_join(customer_threads[i], NULL);
    }

    // Broadcast wake-up signal to any sleeping cooks after all customer orders are in
    pthread_mutex_lock(&(bcb->mutex));
    pthread_cond_broadcast(&(bcb->can_get));
    pthread_mutex_unlock(&(bcb->mutex));

    // Wait for all cook threads to finish
    for (int i = 0; i < NUM_COOKS; i++) {
        pthread_join(cook_threads[i], NULL);
    }

    // 4. Close the restaurant and clean up memory
    CloseRestaurant(bcb);

    return 0;
}