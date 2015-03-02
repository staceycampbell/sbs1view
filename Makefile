CFLAGS := -g -O2 -Wall
LDFLAGS :=
LDLIBS := -lm
LAT := $(shell fgrep BASE_LAT basecoords.h | awk '{ print $$3 }')
LNG := $(shell fgrep BASE_LONG basecoords.h | awk '{ print $$3 }')
RING := .00450450450450450450 \
	.045045045045045045 \
	.09009009009009009 \
	.135135135135135135 \
	.180180180180180180 \
	.225225225225225225 \
	.45045045045045045 \
	.675675675675675675 \
	.9009009009009009

all: sbs1tokml sbs1view circle.dat

sbs1tokml: sbs1tokml.o
	$(CC) $(LDFLAGS) -o sbs1tokml sbs1tokml.o $(LDLIBS)

sbs1view: main.o
	$(CC) $(LDFLAGS) -o sbs1view main.o $(LDLIBS)

sbs1tokml.o main.o circle.dat: basecoords.h

circle.dat:
	rm -f circle.dat
	for i in $(RING); \
	do \
		echo $(LAT) $(LNG) $$i >> circle.dat; \
	done

clean:
	rm -f sbs1tokml.o sbs1tokml sbs1view main.o circle.dat
