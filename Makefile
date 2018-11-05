CC = gcc
obj = dc_common.o dc_main.o

devcfg: $(obj)
	 $(CC) -o $@ $^

dc_common.o: dc_common.h

dc_main.o: dc_common.h

clean:
	rm -f *.o devcfg
