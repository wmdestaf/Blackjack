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
	
	//variable init
	mpz_inits(i, x, y, k, d, n1, ymx, NULL);
	mpz_set_ui(i, 1);
	
	mpz_set(x, start);
	//gmp_printf("%Zd\n", x);
	
	mpz_set(y, x);
	mpz_set_ui(k, 2);
	mpz_set(n1, n);
	mpz_sub_ui(n1,n1,1); //n - 1

	while(1) {
		if(*terminate) {			
			mpz_clears(i,x,y,k,d,ymx,n1,NULL);
			return;
		}
		
		mpz_add_ui(i, i, 1);
		
		//raise
		mpz_powm_ui(x, x, 2, n);
		mpz_sub_ui(x, x, 1); //1 mod n == 1
		if(!mpz_cmp_ui(x, -1)) mpz_set(x, n1);
		
		//gcd
		mpz_sub(ymx, y, x);
		//mpz_abs(ymx, ymx); //gmp will allow neg for gcd
		mpz_gcd(d, ymx, n);
		
		if(mpz_cmp_ui(d, 1) && mpz_cmp(d, n)) {
			mpz_set(res, d);
			mpz_abs(res, res);

			mpz_clears(i,x,y,k,d,ymx,n1,NULL);
			return;
		}
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

	//state init
	gmp_randinit_mt(state);


	#pragma omp parallel for shared(terminate)
	for(i = 0; i < omp_get_num_threads(); ++i) {
		mpz_t start_;
		mpz_init(start_);
		
		#pragma omp critical
		{
			mpz_urandomm(start_, state, n);
		}

		pollard_rho(n, res, start_, &terminate);
		
		#pragma omp critical
		if(!terminate) {
			gmp_printf("%Zd\n",res);
			GET_TIME(end);
			printf("%g seconds\n", end - start);
			exit(1);
			terminate = true;
		}
		mpz_clear(start_);
	}
	
	mpz_clears(n, res, NULL);
	gmp_randclear(state);
	
	GET_TIME(end);
	printf("%g seconds\n", end - start);
	
	return 0;
}