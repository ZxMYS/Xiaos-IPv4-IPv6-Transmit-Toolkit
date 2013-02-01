LINK.o := g++
CPPFLAGS:= -O3
all:proxy
csapp.o:csapp.c csapp.h
main.o:main.cpp csapp.h
proxy:main.o csapp.o
	$(LINK.o) $^ $(LOADLIBES) $(LDLIBS) -o $@
run:proxy
	./proxy
clean:
	rm -f *~ *.o proxy