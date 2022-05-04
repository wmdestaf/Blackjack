#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <sys/time.h>

#include <omp.h>

#define GET_TIME(now) { \
   struct timeval t; \
   gettimeofday(&t, NULL); \
   now = t.tv_sec + t.tv_usec/1000000.0; \
}

#define C_RANGE 255

long int pollard_rho(long, int, int, bool, volatile bool *);

long int gcd(long x, long int y) {
	int t;
	while(y) {
		t = y;
		y = x % y;
		x = t;
	}
	return x;
}

long int g(long x, long c, long int n) {
	return ( (x * x) + c ) % n;
}

long int pollard_rho_parallel(long n, int start, int threads) {
	volatile bool done = false;
	
	
	int i, fin;
	omp_lock_t lock;
	omp_init_lock(&lock);
	
	#pragma omp parallel for shared(done)
	for(i = 0; i < threads; ++i) {
		int res = pollard_rho(n,start + i, 0, true, &done);
		if(res != -3) {
			omp_set_lock(&lock);
			if(!done) {
				fin = res;
				done = true;
			}
			omp_unset_lock(&lock);
		}
	}
	
	omp_destroy_lock(&lock);
	return fin;
}

long pollard_rho(long n,int start,int c, bool retry_on_fail, volatile bool *bail) {
    long x, y, d;
	x = start;
    y = start;
    d = 1;
    
	if(!c) c = 1 + (C_RANGE * rand() / RAND_MAX);

    while(d == 1) {
		if(bail && *bail) return -3;
		
        x = g(x,c,n);
        y = g(g(y,c,n),c,n);
        d = gcd(abs(x - y), n);
	}
        
	if(start == n) return -2;
	else if(d == n) {
		if(retry_on_fail) return pollard_rho(n, start + 1, 0, retry_on_fail, bail);
		return -1;
	}
	return d;
}

int main() {
	long n;
	int res;
	double start, end;
	
	srand(time(NULL));
	n = 397284599;
	
	GET_TIME(start);
	res = pollard_rho_parallel(n,2,1);
	printf("parallel %d\n",res);
	GET_TIME(end);
	printf("elapsed %f\n",end-start);
	
	GET_TIME(start);
	res = pollard_rho(n,2,1,true,NULL);
	printf("parallel %d\n",res);
	GET_TIME(end);
	printf("elapsed %f\n",end-start);
	
	return 0;
}
