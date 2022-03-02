CC             = gcc
CFLAGS         = -O -Wunused
LDFLAGS        = -s

%.o : %.c $(wildcard *.h)
	$(CC) $(CFLAGS) -c $<

objects        		     := $(patsubst %.c,%.o,$(wildcard *.c))


all qx	: $(objects)
	$(CC) $(LDFLAGS) -o qx $(objects) -ltermcap


$(objects)  : $(wildcard *.h)


clean:
	rm -f *.o qx 

install:
	-mv qx /usr/local/bin
