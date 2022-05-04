from math import gcd


FAILURE = -1
def g(x,n):
    return ((x*x)+1) % n
    
def pollard_rho_parallel_naive(n, start=2):
    pass
    
def pollard_rho(n,start=2):
    x = start
    y = start
    d = 1

    while d == 1:
        x = g(x,n)
        y = g(g(y,n),n)
        d = gcd(abs(x - y), n)

    if start == n:
        return -1
    elif d == n: 
        print("Failed on %d, continuing..." % start)
        return pollard_rho(n,start+1)
    else:
        return d

