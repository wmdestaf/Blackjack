from os import system
from pathlib import Path
url = "https://userpages.umbc.edu/~cmarron/cs441.s22/moduli.csv"
new = "mods_new.txt"
old = "mods_old.txt"

if __name__ == "__main__":
    #make old file or not
    if not Path(old).exists():
        system("touch %s" % old)

    #grab file + update differences
    system("curl -s %s >> %s" % (url, new))
    with open(old) as old_f, open(new) as new_f:
        old_lines = old_f.readlines()
        new_lines = new_f.readlines()
    lines = new_lines[len(old_lines):]
    system("mv %s %s" % (new, old))
    
    #send file over to dispatcher
    if not len(lines):
        print("No new moduli!")
        exit(0)