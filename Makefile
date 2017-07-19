CC=g++
CFLAGS=-std=c++11 -I ../yaml-cpp/include/ -O
LDLIBS=-ls3 ../yaml-cpp/build/libyaml-cpp.a -lpthread
PROGRAMS = test_delete test_delete_copy test_get_par test_get_rand test_put create_buckets delete_buckets

all: $(PROGRAMS)
.PHONY: all

test_get_par.o test_get_rand.o test_put.o create_buckets.o delete_buckets.o: util.h

test_delete: test_delete.o util.o

test_delete_copy: test_delete_copy.o util.o

test_get_par: test_get_par.o util.o

test_get_rand: test_get_rand.o util.o

test_put: test_put.o util.o

create_buckets: create_buckets.o util.o

delete_buckets: delete_buckets.o util.o

