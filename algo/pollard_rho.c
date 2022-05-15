//c pollard rho
//bad, will overflow

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

long euclid(long a, long b) {
	long t;
	while(b) {
		t = b;
		b = a % b;
		a = t;
	}
	return a;
}

long pollard_rho(long n) {
	long i, x, y, k, d;
	i = 1;
	x = (n - 1) * ((float)rand()) / RAND_MAX;
	y = x;
	k = 2;
	while(1) {
		i += 1;
		x = ( (x * x) - 1 ) % n;
		d = euclid(y - x, n);
		if( d != 1 && d != n ) return d;
		else if ( i == k ) {
			y = x;
			k <<= 1;
		}
	}	
}

void usage() {
	fprintf(stderr, "%s\n","usage: pollard_rho n");
	exit(1);
}

int main(int argc, char **argv) {
	long n, res;
	
	if(argc != 2 || !argv[1]) usage();
	n = atoi(argv[1]);
	if(n < 3) usage();
	
	srand(time(NULL));
	do {
		res = abs(pollard_rho(n));
	} while(res == 1);
	printf("%lu == %lu * %lu\n", n, res, n / res);
	
	return 0;
}