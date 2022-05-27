#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#define GET_TIME(now) { \
   struct timeval t; \
   gettimeofday(&t, NULL); \
   now = t.tv_sec + t.tv_usec/1000000.0; \
}

#include "mp2.h"

void usage() {
	fprintf(stderr, "%s\n", "usage: pollard_rho_mp2 n");
	exit(1);
}

typedef struct {
	mpz *a, *x, *c, *m;
} randstate_t;

//MMIX's LCG parameters
randstate_t *create_rst(const mpz *seed, int prec) { 
	randstate_t *this = malloc(sizeof(randstate_t));

	load_create(&this->a,"6364136223846793005", prec);
	load_create(&this->c,"1442695040888963407", prec);
	load_create(&this->m,"18446744073709551616",prec);
	this->x = create_mpz(prec);
	if(seed) load(this->x, seed);
	
	return this;
}

void destroy_rst(randstate_t *this) {
	destroy_mpz(this->a);
	destroy_mpz(this->x);
	destroy_mpz(this->c);
	destroy_mpz(this->m);
	free(this);
}

void step(mpz *rop, const mpz *ub, randstate_t *rs) {
	//advance the generator
	mul(rop, rs->a, rs->x);
	add(rop, rop, rs->c);
	idiv(NULL, rs->x, rop, rs->m); //mod
	
	//now, rs->a contains (aX+c) mod m
	//multiply by UB
	mul(rop, rs->x, ub);
	
	//allocate scratch space for division
	mpz *s1 = create_mpz(rop->block_count);
	mpz *s2 = create_mpz(rop->block_count);
	
	//divide by rs->m
	idiv(s1, s2, rop, rs->m);
	load(rop, s1);
	
	destroy_mpz(s1);
	destroy_mpz(s2);
}

void pollard_rho(mpz *rop, const mpz *n, randstate_t *rs) {
	mpz *x, *y, *nn, *s;
	mpz *i, *k;
	
	x  = create_mpz(n->block_count);
	y  = create_mpz(n->block_count);
	nn = create_mpz(n->block_count);
	s  = create_mpz(n->block_count);
	
	step(x, n, rs); //x = Random(0, n - 1)
	load(y, x);
	load(nn,n);
	
	//cycle controls
	i = create_mpz(n->block_count);
	load_ui(i,1,0);
	k = create_mpz(n->block_count);
	load_ui(k,2,0);
	
	while(1) {
		add_ui(i,i,1);
		load(nn,n);
		
		//identity: x + c mod n = (x mod n + c mod n) mod n
		//known 1 mod n == 1, n > 1
		//replace the second mod n with conditional, rolov. zero
		mul(rop,x,x);
		idiv(NULL,x,rop,n);
		add_ui(x,x,1);
		if(compare(x, n) == 0) zero(x);
		
		//y-x
		difference(rop, y, x);
		gcd(rop, nn, s);
		
		//checks
		if(!compare1(rop) && compare(rop, n) != 0) {
			destroy_mpz(x);
			destroy_mpz(y);
			destroy_mpz(nn);
			destroy_mpz(s);
			return;
		}
		if(compare(i, k) == 0) {
			load(y, x);
			ls(k,k,1);
		}
	}
}

int main(int argc, char **argv) {
	if(argc != 2) usage();

	mpz *n, *res;
	randstate_t *rs;
	
	double start, end;
	
	load_create(&n, argv[1], -1);
	res = create_mpz(n->block_count);
	rs = create_rst(NULL, n->block_count);
	
	GET_TIME(start);
	pollard_rho(res, n, rs);
	GET_TIME(end);
	
	printf("%g seconds\n", end - start);
	print(n,1);
	print(res,1);
	
	destroy_rst(rs);
	destroy_mpz(n);
	destroy_mpz(res);

	return 0;
}