#author: Will DeStaffan / w55 / DX00659
#build: none
#run: pollard_rho_python modulus [num_forks]

from math import gcd

#python's GIL makes threading's overhead infeasible here...
from concurrent.futures import ProcessPoolExecutor

from time import time, sleep
from random import randint
from sys import argv

FAILURE = -1
#standard off-cycle evaluation
#x^2 + c mod n expands to (pow(x,2,n) + c mod n) mod n
#can reasonably assume c mod n == n, but double mod == slowww
#therefore, instead use as g(x), x^2 + cx + c^2 mod n.
def g(x,c,n):
    return pow(x+c,2,n) #not the strict x^2 + c % n, 
    
#pollard-rho with master-controller paradigm.
#on one fork's failure, change start, regenerate random offset
# n: modulus
# threads: number of forks
# start: base to start at
#
# returns: first found factor. will loop indefinitely on a prime number.
def pollard_rho_parallel_naive(n, threads, start=2):
    wait = 0.001
    wait_max = 32

    #standard master-controller paradigm
    with ProcessPoolExecutor(max_workers=threads) as executor:
        futures = [executor.submit(pollard_rho, n, start + i, None, False) for i in range(threads)]
        i = start + threads
        while True:
            sleep(wait) #slightly superlinear wait scaling. idea: test often early, less so later
            wait *= 1.5 + wait
            wait = min(wait, wait_max)
            
            for future in futures:
                if not future.done(): #done? if not, don't care
                    continue
                res = future.result() #if done, take result and remove from consideration
                futures.remove(future)
                
                if res == -1: #fail, retry.
                    futures.append(executor.submit(pollard_rho, n, start + i, None, False))
                    i += 1
                else: #success, or is_prime
                    for future in futures:
                        future.cancel()
                    return res
    
#serial pollard-rho 
# n: modulus
# start: number to start at
# c: offset for function g(x) = x^2 + c % n
#
# returns: first found factor. will loop indefinitely on a prime number.
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
        if retry_on_fail: #try again with a different offset
            print("Failed on %d, continuing..." % start)
            return pollard_rho(n,start+1)
        else:
            return -1 #cycle completed without finding anything
    else:
        return d #we've found our factor

def usage():
    print("usage: pollard_rho_python modulus [forks]")
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
        
    if len(argv) == 3: #pull thread count
        #pull thread count
        try:
            t = int(argv[2])
            if t < 1:
                raise ValueError
        except ValueError:
            usage()

        old = time()
        res = pollard_rho_parallel_naive(n,t,start=2)
        new = time()
        print("%d == %d * %d" % (n, res, n / res))
        print("Parallel: %g seconds on %d forks" % (new - old, t))

    else: #serial mode
        old = time()
        res = pollard_rho(n,True)
        new = time()
        print("%d == %d * %d" % (n, res, n / res))
        print("Serial: %g seconds" % (new - old))