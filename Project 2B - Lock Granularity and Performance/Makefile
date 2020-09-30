#NAME: William Chen
#EMAIL: billy.lj.chen@gmail.com
#ID: 405131881

default:
	gcc -Wall -Wextra -g -o lab2_list -lpthread -lprofiler lab2_list.c SortedList.c

tests: default
	chmod +x lab2btests.sh
	-./lab2btests.sh

profile: default
	LD_PRELOAD=/u/ms/ugrad/lijay/Lab2b/lib/libprofiler.so.0
	CPUPROFILE=./temp.gperf ./lab2_list --threads=12 --iterations=1000 --sync=s
	pprof --text ./lab2_list ./temp.gperf > profile.out
	pprof --list=threadMain ./lab2_list ./temp.gperf >> profile.out
	rm -f ./temp.gperf

graphs: tests
	chmod +x lab2bgraphs.gp
	./lab2bgraphs.gp

dist: graphs profile clean
	tar -czvf lab2b-405131881.tar.gz SortedList.h SortedList.c lab2_list.c Makefile lab2b_list.csv profile.out \
	lab2b_1.png lab2b_2.png lab2b_3.png lab2b_4.png lab2b_5.png lab2bgraphs.gp lab2btests.sh README

clean: 
	rm -f lab2b-405131881.tar.gz lab2_list 

