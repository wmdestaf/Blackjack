from sys import argv
from os import system
from pollard_rho_python import pollard_rho 
 
def usage():
    print("usage: pr_base n")
    exit(1)
   
if __name__ == "__main__":
    if len(argv) != 2:
        usage()
    try:
        n = int(argv[1])
        if n < 3:
            raise ValueError
    except ValueError:
        usage()

    res = pollard_rho(n)
    if res == -1:
        print("failed...")
    else:
        print(n,res,int(n/res))
        system("echo \"%d %d %d\" >> facts.txt" % (n,res,int(n/res)))