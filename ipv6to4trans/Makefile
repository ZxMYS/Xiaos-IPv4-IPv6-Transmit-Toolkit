LINK.o := g++
CPPFLAGS := -O3
LINKFLAGS :=
all:6to4trans
csapp.o:csapp.c csapp.h
main.o:main.cpp csapp.h
6to4trans:main.o csapp.o
	$(LINK.o) $(LINKFLAGS) $^ $(LOADLIBES) $(LDLIBS) -o $@
debug: CPPFLAGS := -ggdb
debug: LINKFLAGS := -ggdb
debug:clean all
runtest:6to4
	./6to4 8008 127.0.0.1 80
clean:
	rm -f *~ *.o 6to4trans