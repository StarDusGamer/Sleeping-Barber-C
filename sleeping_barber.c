#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>

//Using unistd.h for it's sleep() function to simulate cutting hair
#include <unistd.h>

int numChairs;
int numCustomers = 0;
#define HAIRCUT_TIME 3

// The pthread mutex variable
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

// The Pthread conditional variables
pthread_cond_t barberReady = PTHREAD_COND_INITIALIZER;
pthread_cond_t customerReady = PTHREAD_COND_INITIALIZER;

int waitingCustomers = 0;
int barberSleeping = 1;
int cuttingHair = 0;

// Barber Function
void* barber(void* arg) {
	while (true) {
		// Only one barber may exist at a time
		pthread_mutex_lock(&lock);

		// If there are no waiting customers, barber sleeps
		while (waitingCustomers == 0) {
			barberSleeping = 1;
			printf("No Customers = Barber is now sleeping...\n");

			// The Barber sleeps till a customer is ready
			pthread_cond_wait(&customerReady, &lock);
		}

		// There are waiting customers, so...
		if (waitingCustomers > 0) {
			waitingCustomers -= 1;

			// Barber wakes up
			barberSleeping = 0;
			printf("Barber is awake = has begun to cut hair...\n");
			printf("Customers now waiting (%d)\n", waitingCustomers);
			cuttingHair = HAIRCUT_TIME;

			// If barber is ready, begins cutting hair
			pthread_cond_signal(&barberReady);
			pthread_mutex_unlock(&lock);

			// While the barber is cutting a customers hair...
			while (cuttingHair > 0) {
				printf("Remaining time for haircut: %d seconds...\n", cuttingHair);
				sleep(1);
				cuttingHair -= 1;
			}
			
			printf("Finished Haircut = Barber starts the next customer...\n");
			// Barber Finishes a customer's hair
		} else {
			printf("Finished Haircut...\n");
			barberSleeping = 1;
			// When haircut is finished, barber gets to sleep
		}
	}
	return NULL;
}

void* customer(void* arg) {
	pthread_mutex_lock(&lock);

	// Customer is allowed in because there is a chair available
	if (waitingCustomers < numChairs) {
		waitingCustomers += 1;
		printf("Customers Waiting (%d)\n", waitingCustomers);
		
		pthread_cond_signal(&customerReady);

		// If barber is sleeping
		while (barberSleeping) {
			// Barber must be ready
			pthread_cond_wait(&barberReady, &lock);
		}

		printf("Barber is Ready...\n\tCuttingHair...\n");
	} else {
		//No chair is available, customer can't wait
		printf("\tNo free chair, Customer Leaves...\n");
	}

	pthread_mutex_unlock(&lock);
	return NULL;
}

int main(int argc, char *argv[]) {
	srand(time(NULL));

	if (argc != 2) {
		fprintf(stderr, "Please input an integer for the # of chairs...\n");
		return 1;
	}

	numChairs = atoi(argv[1]);

	printf("Waiting Chairs (%d)\n", numChairs);

	// Create Pthread for barber and customers
	pthread_t barber_t, customers_t[numCustomers];

	pthread_create(&barber_t, NULL, barber, NULL);


	// Randomly create customers
	while (true) {
		if (rand() % 5 == 0) {
			pthread_t customer_t;
			pthread_create(&customer_t, NULL, customer, NULL);
			printf("Customer Entered...\n\t");
		}
		sleep(1);
	}
}
