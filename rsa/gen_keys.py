#author: Will DeStaffan / w55 / DX00659
#build: none
#exec:  python3 gen_keys.py bits [--safe]
#python modular exp. I assume is just fast modular exp all
#around.

from sys import argv
from random import random, randint, getrandbits

PUB=65537 #per RSA RFC
BASIS =0.35
RADIUS=0.14
SAFE_PRIME=False

#usage message
def usage():
    print("usage: gen_keys bits [--safe]")
    exit(1)
    
#Standard EEA: returns d, and x,y s.t. ax+by=d
#Quite literally the one explained in class
#Normal Euclid, however x and y are backpropogated
#up the call-stack as proven in class.
def extended_euclid(a,b):  
    if b == 0:
        return (a, 1, 0)
    else:
        dd, xx, yy = extended_euclid(b, a % b)
        return(dd, yy, xx - (a // b) * yy )

#splits a number n into t and r, such that
#n - 1 = r * (2^t)
#the algorithm to do so is relatively trivial:
#begin with r = n-1, t = 0. While r is odd, that
#is to say, there exists 2's to be extracted into 
#2^t, a 2 is DIVIDED out, and the t exponent increases
#ADDITIVELY, as is how exponentials work.
def miller_rabin_helper(n):
    r = n - 1
    t = 0
    while r % 2 == 0:
        t += 1
        r >>= 1
        
    return (t, r)
    
#same algo as described in class
#
#Calculate r, t s.t. n - 1 = r * (2 * t)
#for some amount of trials (default for me is 512):
#pick some random base 2 <= a <= n - 2
#repeatedly perform repeated squaring + assignment,
#firing the unity test 't' times 
#then, by the loop's invariant, the repeatedly squared
#variable is nothing more than a^n-1 mod n. If such
#is not 1, it is composite, otherwise continue testing

def miller_rabin(n, k=64):
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

#given a number and a 1d direction (+1, -1)
#attempt to find the next closest prime in that direction.
#should the algorithm start on an even number, it changed
#to the next odd number in the specified direction.
def next_prime(x, direction):
    if x % 2 == 0:
        x += direction

    while not miller_rabin(x):
        x += direction * 2
    return x #is now, likely prime
    
def next_safe_prime(bits, direction, INVERT_RANGE=False, IO=False, IOSTR=''):
    
    off = (BASIS - RADIUS) + (random() * 2 * RADIUS)
    if INVERT_RANGE:
        off = 1 - off
        
    p = getrandbits(int(bits * off) - 1)
    p = max(p,11) #make sure not too small
    p = next_prime(p, direction)
    
    if IO: #maybe having too much fun with this
        anims = ['/','—','\\','|']
        ctr=0
        ccw=0
        wait=5
    
    while True:
        q = (2 * p) + 1
        if miller_rabin(q):
            if IO:
                print('\r'+IOSTR,'DONE')
            return q
        p += 2 * direction
        p = next_prime(p, direction)
        
        if IO:
            ccw = (ccw + 1) % wait
            if not ccw:
                print('\r' + IOSTR + ' ' + anims[ctr], end='')
                ctr = (ctr + 1) % len(anims)

if __name__ == "__main__":
    if len(argv) != 2 and len(argv) != 3:
        usage()
    try:
        bits = int(argv[1])
        if bits < 5:
            print("Too few bits!")
            raise ValueError
    except ValueError:
        usage()

    if len(argv) == 3: 
        if argv[2] == '--safe':
            SAFE_PRIME = True
        else:
            usage()

    if SAFE_PRIME:
        #with such bits, find a reasonable p and q
        p = next_safe_prime(bits, -1, INVERT_RANGE = False, 
                            IO=True, IOSTR='Generating p...')
        q = next_safe_prime(bits,  1, INVERT_RANGE = True, 
                            IO=True, IOSTR='Generating q...')
    else:
        off  = (BASIS - RADIUS) + (random() * 2 * RADIUS)
        off1 = 1 - off 
        p = next_prime(max(5, getrandbits(max(1,int(bits * off )))), -1)
        q = next_prime(max(5, getrandbits(max(1,int(bits * off1)))),  1)

    print(p, q, sep='\n')

    #now, compute N=p*q, and phi(n)=(p-1)*(q-1)
    N = p * q
    phi_n = (p - 1) * (q - 1)
    
    #choose a small odd integer e, relatively prime to phi(n)
    e = PUB
    
    assert extended_euclid(e, phi_n)[0] == 1 #relatively prime

    #compute d, the multiplicative inverse of e, modulo phi(n)
    _, d, _ = extended_euclid(e, phi_n)
    d = d % phi_n

    assert (e * d) % phi_n == 1

    print("N:",N)
    print("PUBLIC  EXPONENT:",e)
    print("PRIVATE EXPONENT:",d)
    
    if SAFE_PRIME:
        print("Your primes are safe!")