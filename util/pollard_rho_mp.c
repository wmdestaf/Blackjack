#include <stdio.h>
#include <stdlib.h>

#include <sys/time.h>

#define GET_TIME(now) { \
   struct timeval t; \
   gettimeofday(&t, NULL); \
   now = t.tv_sec + t.tv_usec/1000000.0; \
}

#include "mp.h"

typedef struct {
	mpz *a, *x, *c, *m, *scratch1, *scratch2;
} randstate_t;

void euclid(mpz *a, const mpz *b, mpz *bb, mpz *scratch1, mpz *scratch2);

//simple LCG
randstate_t *create_randstate() {
	randstate_t *state = malloc(sizeof(randstate_t));
	state->a = create_mpz(64,0);
	state->x = create_mpz(64,0);
	state->c = create_mpz(64,0);
	state->m = create_mpz(64,0);
	state->scratch1 = create_mpz(64,0);
	state->scratch2 = create_mpz(64,0);
	
	load_mpz_str(state->a,"1664525");
	load_mpz_str(state->c,"1013904223");
	load_mpz_str(state->m,"4294967296");
	
	return state;
}

void destroy_randstate(randstate_t *state) {
	destroy_mpz(state->a);
	destroy_mpz(state->x);
	destroy_mpz(state->c);
	destroy_mpz(state->m);
	destroy_mpz(state->scratch1);
	destroy_mpz(state->scratch2);
	free(state);
}

void m_rand(mpz *x, randstate_t *state) {
	//multiply
	mul(state->scratch1, state->a, state->x, state->scratch2);
	
	//add
	add(state->scratch1, state->scratch1, state->c);

	//modulus
	idiv(NULL,state->scratch1, state->m, state->x, state->scratch2);
	
	//store back
	load(x, state->x);
}

void m_randrange_ui(mpz *x, int UB, randstate_t *state) {
	m_rand(x, state);
	
	load_ui(state->scratch1, UB);
	mul(state->scratch2, state->x, state->scratch1, NULL);
	
	//now, divide by 'm'
	idiv(x, state->scratch2, state->m, NULL, NULL);
}

void m_randrange_mp(mpz *x, const mpz *UB, randstate_t *state) {
	m_rand(x, state);

	mul(state->scratch2, state->x, UB, NULL);
	
	//now, divide by 'm'
	idiv(x, state->scratch2, state->m, NULL, NULL);
}

typedef struct {
	
} pollard_rho_state_t;

pollard_rho_state_t *create_pr_state() {
	return NULL;
}

void destroy_pr_state(pollard_rho_state_t *state) {
	
}

void step(pollard_rho_state_t *state) {
	
}

int done(pollard_rho_state_t *state) {
	return 0;
}



void pollard_rho(mpz *n, randstate_t *state) {
	int i = 1, k = 2;
	mpz *n1, *x, *xx, *y, *ymx, *scratch, *scratch2;
	
	//n - 1
	n1 = create_mpz(0, n->block_count);
	load(n1, n);
	dec(n1);
	
	//random x
	x  = create_mpz(0, n->block_count);
	xx = create_mpz(0, n->block_count);
	m_randrange_mp(x, n1, state);

	//y = x
	y = create_mpz(0, n->block_count);
	load(y, x);
	
	//scratch register
	ymx      = create_mpz(0, n->block_count);
	scratch  = create_mpz(0, n->block_count);
	scratch2 = create_mpz(0, n->block_count);
	
	while(1) {
		i++;
		mul(xx, x, x, scratch);
		idiv(NULL, xx, n , x, scratch); //(x^2) % n
		
		//'mod' on 0, else 'mod'
		if(compare0(x))
			load(x, n1); //-1 mod n = n - 1
		else
			dec(x); //minus 1

		//ymx = y - x
		abs_sub(ymx, y, x);

		//euclidean gcd (note: bricks 'a'!)
		euclid(ymx, n, xx, scratch, scratch2);

		if(!compare1(ymx) && compare(ymx,n) != 0) {
			load(n, ymx);
			return;
		}
		if(i == k) {
			load(y, x);
			k <<= 1;
		}
	}
}

/*
	function gcd(a, b)
		while b ≠ 0
			t := b
			b := a mod b
			a := t
		return a
*/
void euclid(mpz *a, const mpz *b, mpz *bb, mpz *scratch1, mpz *scratch2) {
	load(bb, b);
	
	while(!compare0(bb)) {
		idiv(NULL, a, bb, scratch1, scratch2);
		swap(scratch1, bb);
		swap(scratch1, a);
	}
}

int main(int argc, char **argv) {
	mpz *n;
	randstate_t *state;
	double start, end;
	
	GET_TIME(start);
	state = create_randstate();
	n = create_mpz(2 * safe_bit_ct(argv[1]), 0);
	load_mpz_str(n, argv[1]);
	
	printf("Loaded: ");
	fflush(NULL);
	print_mpz(n);

	pollard_rho(n, state);
	printf("Found Factor: ");
	fflush(NULL);
	print_mpz(n);
	GET_TIME(end);
	printf("%g seconds\n", end - start);
}







