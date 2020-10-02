# NAME: William Chen
# EMAIL: billy.lj.chen@gmail.com
# ID: 405131881

default: 
	gcc  -Wall -Wextra -lmraa -lm -g lab4c_tcp.c -o lab4c_tcp
	gcc  -Wall -Wextra -lmraa -lm -g -lssl -lcrypto lab4c_tls.c -o lab4c_tls

clean:
	rm -f lab4c-405131881.tar.gz lab4c_tcp lab4c_tls

dist:
	tar -czvf lab4c-405131881.tar.gz lab4c_tcp.c lab4c_tls.c Makefile README