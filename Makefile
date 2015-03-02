CFLAGS := -g -O2 -Wall
LDFLAGS :=
LDLIBS := -lm

all: sbs1tokml sbs1view

sbs1tokml: sbs1tokml.o
	$(CC) $(LDFLAGS) -o sbs1tokml sbs1tokml.o $(LDLIBS)

sbs1view: main.o
	$(CC) $(LDFLAGS) -o sbs1view main.o $(LDLIBS)

sbs1tokml.o main.o: basecoords.h

clean:
	rm -f sbs1tokml.o sbs1tokml sbs1view main.o
