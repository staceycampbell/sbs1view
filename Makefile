CFLAGS := -g -O2 -Wall
LDFLAGS :=
LDLIBS := -lm

sbs1view: main.o
	$(CC) $(LDFLAGS) -o sbs1view main.o $(LDLIBS)
