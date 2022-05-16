#include <stdlib.h>

#include "mp.h"

/*
i = 1
x = Random(0, n − 1)
y = x
k = 2
while True
	i = i + 1
	x = (x^2 - 1) mod n
	d = gcd(y − x, n)
	if d != 1 and d != n
		return d
	if i == k
		y = x
		k <<= 1
*/

typedef struct {
	mpz *a, *x, *c, *m, *scratch1, *scratch2;
} randstate_t;

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

int main(int argc, char **argv) {
	mpz *a;
	randstate_t *state;
	
	a = create_mpz(64,0);
	state = create_randstate();

	int i;
	for(i = 0; i < 5; ++i) {
		m_rand(a, state);
		print_mpz(a);
	}
}







