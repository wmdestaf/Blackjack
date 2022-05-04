from math import gcd
from concurrent.futures import ProcessPoolExecutor
from time import time, sleep
from random import randint

FAILURE = -1
def g(x,c,n):
    return ((x*x)+1) % n
    
def pollard_rho_parallel_naive(n, threads, start=2):
    #start 'thread' threads from 2 to 2+threads
    with ProcessPoolExecutor(max_workers=threads) as executor:
        futures = [executor.submit(pollard_rho, n, start + i, None, False) for i in range(threads)]
        i = start + threads
        while True:
            sleep(2)
            for future in futures:
                if not future.done(): #done?
                    continue
                res = future.result() #cleanup
                futures.remove(future)
                
                if res == -1: #fail, retry.
                    futures.append(executor.submit(pollard_rho, n, start + i, None, False))
                    i += 1
                else: #success, or is_prime
                    for future in futures:
                        future.cancel()
                    return res
    
    
def pollard_rho(n,start=2,c=1, retry_on_fail=True):
    x = start
    y = start
    d = 1
    
    if c == None:
        c = randint(1,255)

    while d == 1:
        x = g(x,c,n)
        y = g(g(y,c,n),c,n)
        d = gcd(abs(x - y), n)
        
    if start == n: #prime!!
        return -2 
    elif d == n:
        if retry_on_fail:
            print("Failed on %d, continuing..." % start)
            return pollard_rho(n,start+1)
        else:
            return -1
    else:
        return d

if __name__ == "__main__":
    n = 98834976202698839303077
    #n=17
    serial = True
    
    old = time()
    print(pollard_rho_parallel_naive(n,1,start=2))
    new = time()
    print("Parallel: %g" % (new - old))
    
    old = time()
    print(pollard_rho(n,True))
    new = time()
    print("Serial: %d" % (new - old))
    
    
    
    
    
    
    