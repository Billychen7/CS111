#! /usr/bin/gnuplot
#NAME: William Chen
#ID: 405131881
#EMAIL: billy.lj.chen@gmail.com
# input: lab2b_list.csv

set terminal png
set datafile separator ","

#1
set title "Throughput vs. Number of Threads for Mutex and Spin-lock Synchronized"
set xlabel "# of Threads"
set logscale x 2
set xrange [0.75:]
set ylabel "Throughput (ops/sec)"
set logscale y 10
set output 'lab2b_1.png'
plot \
    "< grep -e 'list-none-s,[0-9]*,1000,1,' lab2b_list.csv" using ($2):(1000000000/($7)) \
	title 'Spin Lock' with linespoints lc rgb 'red', \
    "< grep -e 'list-none-m,[0-9]*,1000,1,' lab2b_list.csv" using ($2):(1000000000/($7)) \
	title 'Mutex Lock' with linespoints lc rgb 'blue'

#2 
set title "Mean Time per Mutex Wait and Mean Time per Operation"
set xlabel "# of Threads"
set logscale x 2
set xrange [0.75:]
set ylabel "Time per op (ns)"
set logscale y 10
set output 'lab2b_2.png'
plot \
    "< grep -e 'list-none-m,[0-9]*,1000,1,' lab2b_list.csv" using ($2): ($8) \
	title 'wait_for_lock' with linespoints lc rgb 'red', \
    "< grep -e 'list-none-m,[0-9]*,1000,1,' lab2b_list.csv" using ($2): ($7) \
	title 'time_per_op' with linespoints lc rgb 'blue'
    

#3
set title "Successful Iterations vs. Threads"
set xlabel "# of Threads"
set logscale x 2
set xrange [0.75:]
set ylabel "Successful Iterations"
set logscale y 10
set output 'lab2b_3.png'
plot \
    "< grep 'list-id-none,[0-9]*,[0-9]*,4,' lab2b_list.csv" using ($2):($3) \
	title 'Unprotected' with points lc rgb 'red', \
    "< grep 'list-id-s,[0-9]*,[0-9]*,4,' lab2b_list.csv" using ($2):($3) \
	title 'Spin Lock' with points lc rgb 'blue', \
    "< grep 'list-id-m,[0-9]*,[0-9]*,4,' lab2b_list.csv" using ($2):($3) \
	title 'Mutex Lock' with points lc rgb 'green' 

#4
set title "Throughput vs. Number of Threads for Mutex Locks"
set xlabel "# of Threads"
set logscale x 2
set xrange [0.75:]
set ylabel "Throughput (ops/sec)"
set logscale y 10
set yrange [10000:]
set output 'lab2b_4.png'
plot \
     "< grep 'list-none-m,[0-9]*,1000,1,' lab2b_list.csv" using ($2):(1000000000/($7)) \
	title 'Mutex, Lists = 1' with linespoints lc rgb 'red', \
     "< grep 'list-none-m,[0-9]*,1000,4,' lab2b_list.csv" using ($2):(1000000000/($7)) \
	title 'Mutex, Lists = 4' with linespoints lc rgb 'blue', \
     "< grep 'list-none-m,[0-9]*,1000,8,' lab2b_list.csv" using ($2):(1000000000/($7)) \
	title 'Mutex, Lists = 8' with linespoints lc rgb 'green', \
     "< grep 'list-none-m,[0-9]*,1000,16,' lab2b_list.csv" using ($2):(1000000000/($7)) \
	title 'Mutex, Lists = 16' with linespoints lc rgb 'orange'

#5
set title "Throughput vs. Number of Threads for Spin Locks"
set xlabel "# of Threads"
set logscale x 2
set xrange [0.75:]
set ylabel "Throughput (ops/sec)"
set logscale y 10
set yrange [10000:]
set output 'lab2b_5.png'
plot \
     "< grep 'list-none-s,[0-9]*,1000,1,' lab2b_list.csv" using ($2):(1000000000/($7)) \
	title 'Spin, Lists = 1' with linespoints lc rgb 'red', \
     "< grep 'list-none-s,[0-9]*,1000,4,' lab2b_list.csv" using ($2):(1000000000/($7)) \
	title 'Spin, Lists = 4' with linespoints lc rgb 'blue', \
     "< grep 'list-none-s,[0-9]*,1000,8,' lab2b_list.csv" using ($2):(1000000000/($7)) \
	title 'Spin, Lists = 8' with linespoints lc rgb 'green', \
     "< grep 'list-none-s,[0-9]*,1000,16,' lab2b_list.csv" using ($2):(1000000000/($7)) \
	title 'Spin, Lists = 16' with linespoints lc rgb 'orange'