LINK.o := g++
CPPFLAGS := -O3
LINKFLAGS :=
all:4to6trans
csapp.o:csapp.c csapp.h
main.o:main.cpp csapp.h
4to6trans:main.o csapp.o
	$(LINK.o) $(LINKFLAGS) $^ $(LOADLIBES) $(LDLIBS) -o $@
debug: CPPFLAGS := -ggdb
debug: LINKFLAGS := -ggdb
debug:clean all
runtest:4to6trans
	./4to6trans 8008 2404:6800:8005::93 80
clean:
	rm -f *~ *.o 4to6trans