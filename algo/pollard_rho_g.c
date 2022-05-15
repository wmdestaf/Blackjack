//c pollard rho, with gmp

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <gmp.h>
#include <omp.h>

void usage() {
	fprintf(stderr, "%s\n","usage: pollard_rho n");
	exit(1);
}
	
void pollard_rho(const mpz_t n, mpz_t res, volatile bool *terminate) {
	mpz_t i, x, y, k, d, ymx;
	gmp_randstate_t state;
	
	//state init
	gmp_randinit_mt(state);
	
	//variable init
	mpz_inits(i, x, y, k, d, ymx, NULL);
	mpz_set_ui(i, 1);
	mpz_urandomm(x, state, n);
	mpz_set(y, x);
	mpz_set_ui(k, 2);

	while(1) {
		#pragma omp flush(terminate)
		if(*terminate) return;
		
		mpz_add_ui(i, i, 1);
		
		//raise
		mpz_powm_ui(x, x, 2, n);
		mpz_sub_ui(x, x, 1);
		mpz_mod(x, x, n);
		
		//gcd
		mpz_sub(ymx, y, x);
		mpz_gcd(d, ymx, n);
		
		if(mpz_cmp_ui(d, 1) && mpz_cmp(d, n)) {
			mpz_set(res, d);
			mpz_abs(res, res);
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
	int i;
	volatile bool terminate;

	if(argc != 2 || !argv[1]) usage();
	mpz_init_set_str(n, argv[1], 10);
	mpz_init(res);

	#pragma omp parallel for
	for(i = 0; i < omp_get_num_threads(); ++i) {
		pollard_rho(n, res, &terminate);
		
		#pragma omp critical
		if(!terminate) {
			gmp_printf("%Zd\n",res);
			terminate = true;
		}
	}
	
	mpz_clear(n);
	mpz_clear(res);
	
	return 0;
}