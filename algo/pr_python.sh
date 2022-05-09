#!/bin/bash
FORKS=4
SCRIPT="pollard_rho_python.py"
raw=$(cat given_mods.txt | head -n 8 | tail -n 7 | tr "\n," " ")
read -a lines <<< "$raw"

for ((i=0;i<=${#lines[@]};i+=2)); do
	echo "Testing modulus: " ${lines[$i]}
	python "$SCRIPT" "${lines[$((i+1))]}" "$FORKS"
done