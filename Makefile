#NAME: William Chen
#EMAIL: billy.lj.chen@gmail.com
#ID: 405131881

default:
	gcc -o lab1a -g -Wall -Wextra lab1a.c

clean:
	rm -f lab1a lab1a-405131881.tar.gz

dist:
	tar -cvzf lab1a-405131881.tar.gz lab1a.c README Makefile
