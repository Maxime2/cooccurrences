CFLAGS=-O3 -funsigned-char

all: a.out b.out

a.out: a.in a.txt cooccur
	@echo
	@echo Example 1
	./cooccur a.txt 2 < a.in | tee a.out
	@echo

cooccur: cooccur.c Makefile
	$(CC) $(CFLAGS) cooccur.c -o cooccur -lm

b.out: b.in b.txt cooccur
	@echo
	@echo Example 2
	./cooccur b.txt 3 < b.in | tee b.out
	@echo

clean:
	rm cooccur

