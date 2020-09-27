#NAME: William Chen
#EMAIL: billy.lj.chen@gmail.com
#ID: 405131881

default:
	gcc -g -Wall -Wextra -o lab0 lab0.c

check: default test

test: noArgTest invalidArgTest redirectTest inputTest outputTest catchTest 
	if [ -s smokeTest.txt ]; then \
		echo "Errors include: "; \
		cat smokeTest.txt; \
	else \
		echo "Smoke Test passed"; \
	fi
	rm -f smokeTest.txt; \

noArgTest:
	#check for correct exit code when no argument is passed
	echo "input" > input.txt; \
	./lab0 < input.txt > output.txt
	if [ $$? -ne 0 ]; then \
		echo "Program did not end with exit code 0"; \
	fi
	rm -f output.txt

invalidArgTest:
	#check for correct exit code if invalid argument is passed
	./lab0 --invalidArg 2> tmp.txt; \
	if [ $$? -ne 1 ]; then \
		echo "Wrong exit code for invalid argument" >> smokeTest.txt; \
	fi
	if [ ! -s tmp.txt ]; then \
		echo "Missing error messsage for invalid argument" >> smokeTest.txt; \
	fi
	rm -f tmp.txt \

redirectTest:
	#check correct results and exit code
	echo "testing io" > input4.txt; \
	./lab0 --input input4.txt --output output4.txt; \
	if [ $$? -ne 0 ]; then \
		echo "Wrong exit code for --input --output" >> smokeTest.txt; \
	fi
	cmp -s input4.txt output4.txt; \
	if [ $$? -eq 1 ]; then \
		echo "Failure with input/output redirection" >> smokeTest.txt; \
	fi

	rm -f input4.txt output4.txt; \

inputTest:
	echo "input" > input.txt; \

	#file does not exist
	./lab0 --input thisFileDoesNotExist.txt 2> output1.txt; \
	if [ $$? -ne 2 ]; then \
		echo "Wrong exit code for file does not exist for --input" >> smokeTest.txt; \
	fi
	if [ ! -s output1.txt ]; then \
		echo "Missing error message for file does not exist for --input" >> smokeTest.txt; \
	fi
	#argument called without actual input
	./lab0 --input 2> output2.txt; \
	if [ $$? -ne 1 ]; then \
		echo "Wrong exit code for missing argument for --input" >> smokeTest.txt; \
	fi
	if [ ! -s output2.txt ]; then \
		echo "Missing error message for missing argument for --input" >> smokeTest.txt; \
	fi
	#correct input
	./lab0 --input input.txt > output3.txt; \
	if [ $$? -ne 0 ]; then \
		echo "Wrong exit code for --input" >> smokeTest.txt; \
	fi
	cmp -s input.txt output3.txt; \
	if [ $$? -eq 1 ]; then \
		echo "Failed input redirection for --input" >> smokeTest.txt; \
	fi
	rm -f output1.txt output2.txt output3.txt input.txt; \

outputTest:
	echo "input" > input.txt; \
	
	#checking for error message failing creat(2)
	touch tmp.txt; \
	 chmod -w tmp.txt; \
	cat input.txt | ./lab0 --output tmp.txt 2> output1.txt; \
	if [ $$? -ne 3 ]; then \
	   	echo "Wrong exit code for invalid --output" >> smokeTest.txt; \
	fi
	if [ ! -s output1.txt ]; then \
	   	echo "Missing error message for invalid --output" >> smokeTest.txt; \
	fi
	#argument called without actual output 
	./lab0 --output 2> output2.txt; \
	if [ $$? -ne 1 ]; then \
		echo "Wrong exit code for missing argument for --output" >> smokeTest.txt; \
	fi
	if [ ! -s output2.txt ]; then \
		echo "Missing error message for missing argument for --output" >> smokeTest.txt; \
	fi
	#correct output
	cat input.txt | ./lab0 --output output3.txt; \
	if [ $$? -ne 0 ]; then \
		echo "Wrong exit code for --output" >> smokeTest.txt; \
	fi
	cmp -s input.txt output3.txt; \
	if [ $$? -eq 1 ]; then \
		echo "Failed output redirection for --output" >> smokeTest.txt; \
	fi
	rm -f output1.txt output2.txt output3.txt input.txt tmp.txt; \

catchTest:
	#catch seg fault and return correct exit code and error message
	./lab0 --segfault --catch 2> tmp.txt; \
	if [ $$? -ne 4 ]; then \
		echo "Wrong exit code for --segfault --catch" >> smokeTest.txt; \
	fi
	if [ ! -s tmp.txt ]; then \
		echo "Missing error message for --segfault --catch" >> smokeTest.txt; \
	fi
	rm -f tmp.txt \

dist:
	tar -czvf lab0-405131881.tar.gz lab0.c backtrace.png breakpoint.png Makefile README 

clean:
	rm -f lab0 *.txt *.tar.gz *.o
