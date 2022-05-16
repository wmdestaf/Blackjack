from sys import argv
from sympy import isprime
from math import isqrt, log, floor, gcd

def logk(x, k):
    return log(x) / log(k)

def pollard_p1_internal(n, B, primes):
    B_idx = binsearch_key_ceil(B, primes)
    M = 1
    for idx in range(0, B_idx):
        prime = primes[idx]
        M *= pow(prime,floor(logk(B,prime))) 
        
    t = (pow(2, M, n) - 1) % n
    d = gcd(t, n)
    return d

def binsearch_key_ceil(k, what):
    idx = len(what) >> 1
    dif = idx
    while True:
        if what[idx - 1] < k and what[idx] >= k:
            return idx
        elif what[idx] < k: #too low
            idx += dif
        else: #what[idx] < k, too high
            idx -= dif
        
        dif = max(dif >> 1, 1)
            
    return idx

def pollard_p1(n):
    primes = [x for x in range(2, n + 1) if isprime(x)]
    B = int(log(n))
    window = isqrt(n)
    
    while True:
        res = pollard_p1_internal(n, B, primes)

        if res == 1:
            B += max(1, B / window) #ensure dB does not get too small
        elif res == n:
            B -= max(1, B / window)
        else:
            return res
        
        window += 1
        

def usage():
    print('pollard_p1 n')
    exit(1)

if __name__ == "__main__":
    if len(argv) != 2 and len(argv) != 3:
        usage()
    #pull modulus
    try:
        n = int(argv[1])
        if n < 2:
            raise ValueError
    except ValueError:
        usage()
        
    res = pollard_p1(n)
    print(n, res, n // res)