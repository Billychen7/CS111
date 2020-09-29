#!/bin/bash

# NAME: William Chen
# EMAIL: billy.lj.chen@gmail.com
# UID: 405131881


#add-none
for i in 1, 2, 4, 8, 12
do
    for j in 10, 20, 40, 80, 100, 1000, 10000, 100000
    do
        ./lab2_add --iterations=$j --threads=$i >> lab2_add.csv
    done
done


#add-yield-none  
for i in 1, 2, 4, 8, 12
do
        for j in 10, 20, 40, 80, 100, 1000, 10000, 100000
        do
            ./lab2_add --iterations=$j --threads=$i --yield >> lab2_add.csv
        done
done

#add-m  
for i in 1, 2, 4, 8, 12
do
        for j in 10, 20, 40, 80, 100, 1000, 10000, 100000
        do
            ./lab2_add --iterations=$j --threads=$i --sync=m >> lab2_add.csv
        done
done

#add-s  
for i in 1, 2, 4, 8, 12
do
        for j in 10, 20, 40, 80, 100, 1000, 10000, 100000
        do
            ./lab2_add --iterations=$j --threads=$i --sync=s >> lab2_add.csv
        done
done

#add-c  
for i in 1, 2, 4, 8, 12
do
        for j in 10, 20, 40, 80, 100, 1000, 10000, 100000
        do
            ./lab2_add --iterations=$j --threads=$i --sync=c >> lab2_add.csv
        done
done

#add-yield-m  
for i in 1, 2, 4, 8, 12
do
        for j in 10, 20, 40, 80, 100, 1000, 10000, 100000
        do
            ./lab2_add --iterations=$j --threads=$i --yield --sync=m >> lab2_add.csv
        done
done

#add-yield-s  
for i in 1, 2, 4, 8, 12
do
        for j in 10, 20, 40, 80, 100, 1000, 10000, 100000
        do
            ./lab2_add --iterations=$j --threads=$i --yield --sync=s >> lab2_add.csv
        done
done

#add-yield-c  
for i in 1, 2, 4, 8, 12
do
        for j in 10, 20, 40, 80, 100, 1000, 10000, 100000
        do
            ./lab2_add --iterations=$j --threads=$i --yield --sync=c >> lab2_add.csv
        done
done