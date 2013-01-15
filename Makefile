randpass: randpass.c hudatabase.c hudatabase.h
	#gcc -Wall -g -o randpass -DDD_DEBUG randpass.c hudatabase.c -lddutil-dbg -lpthread
	gcc -Wall -O2 -o randpass randpass.c hudatabase.c -lddutil -lpthread

hudatabase.c: hudatabase.h

hudatabase.h: Huff.dd
	datadraw Huff.dd
