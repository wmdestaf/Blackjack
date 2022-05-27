#run: pollard_p1 modulus

from sys import argv
from math import isqrt, log, floor, gcd
from random import randint

#max look?
def tune(n):
    return int( log(n) + 1 )

def miller_rabin_helper(n):
    r = n - 1
    t = 0
    while r % 2 == 0:
        t += 1
        r >>= 1
        
    return (t, r)
    
def isprime(n, k=64):
    t, r = miller_rabin_helper(n)

    for __ in range(k):
        a = randint(2, n - 2)
        b = pow(a,r,n)
        for _ in range(1, t + 1):
            c = pow(b,2,n)
            if c == 1 and (b != 1 % n and b != n - 1 % n):
                return False #most definitely
            b = c
        if b != 1:
            return False #most definitely
    return True #maybe

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
    primes = [x for x in range(5, 3 * tune(n),2) if isprime(x)]
    print("Generated....")
    
    B = tune(n)
    window = 1
    
    while True:
        res = pollard_p1_internal(n, B, primes)

        if res == 1:
            B += max(1, B / window) #ensure dB does not get too small
        elif res == n:
            B -= max(1, B / window)
        else:
            print("Succeeded on bound",B)
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
    print(n, res, n // res,sep='\n\n')