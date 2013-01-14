randpass: randpass.c hudatabase.c hudatabase.h
	#gcc -Wall -g -o huff -DDD_DEBUG huff.c hudatabase.c -lddutil-dbg
	gcc -Wall -O2 -o randpass randpass.c hudatabase.c -lddutil -lpthread

hudatabase.c: hudatabase.h

hudatabase.h: Huff.dd
	datadraw Huff.dd
