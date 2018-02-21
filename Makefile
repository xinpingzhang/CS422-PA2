
CC = gcc
OPT = -O0
BUILDDIR = build
BINDIR = bin

all: CREATE_DIR $(BINDIR)/prog2_arq $(BINDIR)/prog2_gbn

CREATE_DIR:
	mkdir -p $(BUILDDIR) $(BINDIR)


$(BUILDDIR)/prog2.o: prog2.c
	$(CC) -g $(OPT) -c prog2.c -o $(BUILDDIR)/prog2.o

$(BUILDDIR)/prog2_arq.o: prog2_arq.c prog2.h
	$(CC) -g $(OPT) -c prog2_arq.c -o $(BUILDDIR)/prog2_arq.o

$(BUILDDIR)/prog2_gbn.o: prog2_gbn.c prog2.h
	$(CC) -g $(OPT) -c prog2_gbn.c -o $(BUILDDIR)/prog2_gbn.o

$(BINDIR)/prog2_arq: $(BUILDDIR)/prog2_arq.o $(BUILDDIR)/prog2.o
	$(CC) -g $(OPT) $(BUILDDIR)/prog2_arq.o $(BUILDDIR)/prog2.o -o $(BINDIR)/prog2_arq

$(BINDIR)/prog2_gbn: $(BUILDDIR)/prog2_gbn.o $(BUILDDIR)/prog2.o
	$(CC) -g $(OPT) $(BUILDDIR)/prog2_gbn.o $(BUILDDIR)/prog2.o -o $(BINDIR)/prog2_gbn

clean:
	rm -rf $(BINDIR) $(BUILDDIR)
