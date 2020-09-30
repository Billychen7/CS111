#!/bin/bash
#NAME: William Chen
#EMAIL: billy.lj.chen@gmail.com
#ID: 405131881

rm -f lab2b_list.csv

for i in 1, 2, 4, 8, 12, 16, 24
do
	for j in m, s
    do
		./lab2_list --threads=$i --iterations=1000 --sync=$j >> lab2b_list.csv
	done
done

for i in 1, 4, 8, 12, 16
do
	for j in 1, 2, 4, 8, 16
	do
		./lab2_list  --threads=$i --iterations=$j --yield=id --lists=4 >> lab2b_list.csv
	done
done

for i in 1, 4, 8, 12, 16
do
    for j in 10, 20, 40, 80
    do
        for k in m, s
        do
            ./lab2_list --threads=$i --iterations=$j --yield=id --sync=$k --lists=4 >> lab2b_list.csv
        done
    done
done

for i in 1, 2, 4, 8, 12
do
    for j in 4, 8, 16
    do
        for k in m, s
        do
            ./lab2_list --threads=$i --iterations=1000 --sync=$k --lists=$j >> lab2b_list.csv
        done
    done
done