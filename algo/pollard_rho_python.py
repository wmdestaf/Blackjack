from math import gcd
from concurrent.futures import ProcessPoolExecutor
from time import time, sleep

FAILURE = -1
def g(x,n):
    return ((x*x)+1) % n
    
def pollard_rho_parallel_naive(n, threads, start=2):
    #start 'thread' threads from 2 to 2+threads
    with ProcessPoolExecutor(max_workers=threads) as executor:
        futures = [executor.submit(pollard_rho, n, start + i, False) for i in range(threads)]
        i = start + threads
        while True:
            sleep(2)
            for future in futures:
                if not future.done(): #done?
                    continue
                res = future.result() #cleanup
                futures.remove(future)
                
                if res == -1: #fail, retry.
                    futures.append(executor.submit(pollard_rho, n, start + i, False))
                    i += 1
                else: #success, or is_prime
                    for future in futures:
                        future.cancel()
                    return res
    
    
def pollard_rho(n,start=2,retry_on_fail=True):
    x = start
    y = start
    d = 1

    while d == 1:
        x = g(x,n)
        y = g(g(y,n),n)
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
    n = 410082333130306064848248112591
    #n=17
    serial = True
    
    old = time()
    print(pollard_rho_parallel_naive(n,2,start=2))
    new = time()
    print("Parallel: %g" % (new - old))
    
    old = time()
    print(pollard_rho(n,True))
    new = time()
    print("Serial: %d" % (new - old))
    
    
    
    
    
    
    