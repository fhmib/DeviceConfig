CC = gcc
obj = dc_common.o dc_main.o
CFLAGS = -g

devcfg: $(obj)
	$(CC) $(CFLAGS)  $^ -o $@ -lpthread

dc_common.o: dc_common.h

dc_main.o: dc_common.h

clean:
	rm -f *.o devcfg
