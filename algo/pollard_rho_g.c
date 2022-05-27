//compile: 	  gcc pollard_rho_g.c -o pollard_rho_g -lgmp
//compile MT: gcc pollard_rho_g.c -o pollard_rho_g -lgmp -fopenmp
//run:        ./pollard_rho_g modulus

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <sys/time.h>

#include <gmp.h>
#include <omp.h>

#define GET_TIME(now) { \
   struct timeval t; \
   gettimeofday(&t, NULL); \
   now = t.tv_sec + t.tv_usec/1000000.0; \
}

#if defined(_OPENMP)
#include <omp.h>
#else
inline int omp_get_num_threads() { return 1;}
#endif

void usage() {
	fprintf(stderr, "%s\n","usage: pollard_rho n");
	exit(1);
}
	
void pollard_rho(const mpz_t n, mpz_t res, mpz_t start, volatile bool *terminate) {
	mpz_t i, x, y, k, d, n1, ymx;
	
	//make space for variables
	mpz_inits(i, x, y, k, d, n1, ymx, NULL);
	
	//initialize variables
	mpz_set_ui(i, 1);
	mpz_set(x, start);
	mpz_set(y, x);
	mpz_set_ui(k, 2);
	
	//store n-1, for comparison
	mpz_set(n1, n);
	mpz_sub_ui(n1,n1,1);

	while(1) {
		if(*terminate) { //once a thread has terminated, don't care about result
		                 //even without CRCW, an incoherent read will hit this on next loop
			mpz_clears(i,x,y,k,d,ymx,n1,NULL);
			return;
		}
		
		//inc i
		mpz_add_ui(i, i, 1);

		/**
			identity: (a + b) mod n = ((a mod n) + (b mod n)) mod n
			know that 1 mod n == 1.
			also, for x - k mod n, can represent as piecewise:
			
			(x - k) mod n = x - k,     k <= x
			              = n - k - x, k  > x
							
			for x - 1 mod n, can generalize further:
			
			(x - 1) mod n = x - 1,     x != 0
                          = n - 1,     x == 0
			
			Saving 1 modulus per cycle.
		*/
		
		//x = x^2 - 1 mod n
		mpz_powm_ui(x, x, 2, n);
		mpz_sub_ui(x, x, 1);
		if(!mpz_cmp_ui(x, -1)) mpz_set(x, n1);
		
		//d = gcd(y-x, n)
		mpz_sub(ymx, y, x);
		mpz_gcd(d, ymx, n);
		
		//if d is not trivial, return our result
		if(mpz_cmp_ui(d, 1) && mpz_cmp(d, n)) {
			mpz_set(res, d);
			mpz_abs(res, res);

			mpz_clears(i,x,y,k,d,ymx,n1,NULL); //cleanup
			return;
		}
		
		//otherwise if our 'rho' has cycled, reconfigure parameters
		else if(!mpz_cmp(i, k)) {
			mpz_set(y, x);
			mpz_mul_2exp(k, k, 1);
		}
	}
}
	
int main(int argc, char **argv) {
	mpz_t n, res;
	gmp_randstate_t state;
	double start, end;
	int i;
	volatile bool terminate;

	if(argc != 2 || !argv[1]) usage();
	mpz_init_set_str(n, argv[1], 10);
	mpz_init(res);
	
	GET_TIME(start)
	terminate = false;
	gmp_randinit_mt(state);


	#pragma omp parallel for shared(terminate)
	for(i = 0; i < omp_get_num_threads(); ++i) { //PR is embarrasingly parallel, oversubscribing is pointless here.
		mpz_t start_;
		mpz_init(start_);
		
		#pragma omp critical
		//make sure each thread has their own random num from the LCG
		{
			mpz_urandomm(start_, state, n);
		}

		pollard_rho(n, res, start_, &terminate);
		
		#pragma omp critical
		if(!terminate) {
			gmp_printf("%Zd\n",res);
			GET_TIME(end);
			terminate = true; //set flag exactly once
		}
		mpz_clear(start_);
	}
	
	mpz_clears(n, res, NULL);
	gmp_randclear(state);
	
	GET_TIME(end);
	printf("%g seconds\n", end - start);
	
	return 0;
}