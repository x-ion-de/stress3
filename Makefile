# Copyright 2017 x-ion GmbH
CXX      = g++
CPPFLAGS = -std=c++11 -I src/yaml-cpp/include -I src/ratelimiter/src -g -Wall
LDLIBS   = -ls3 -lpthread
LDFLAGS  = -g
PROGRAMS = test_copy test_delete test_delete_copy test_get test_get_par test_get_rand test_put create_buckets delete_buckets list_buckets_rand
BINDIR   = bin
SRCDIR   = src
OBJDIR   = obj
TARGETS := $(PROGRAMS:%=$(BINDIR)/%)

all: $(TARGETS)
.PHONY: all

SOURCES  := $(wildcard $(SRCDIR)/*.cc)
INCLUDES := $(wildcard $(SRCDIR)/*.h)
OBJECTS  =  $(OBJDIR)/util.o $(OBJDIR)/rate_limiter.o $(OBJDIR)/libyaml-cpp.a

ALLOBJECTS := $(SOURCES:$(SRCDIR)/%.cc=$(OBJDIR)/%.o)

$(OBJDIR)/rate_limiter.o: src/ratelimiter/src/rate_limiter.cpp src/ratelimiter/src/*.hpp
	$(CXX) $(CPPFLAGS) -c $< -o $@

$(OBJDIR)/libyaml-cpp.a:
	mkdir -p src/yaml-cpp/build
	cd src/yaml-cpp/build; cmake ..
	$(MAKE) -C src/yaml-cpp/build
	mv src/yaml-cpp/build/libyaml-cpp.a $(OBJDIR)

$(ALLOBJECTS): $(OBJDIR)/%.o : $(SRCDIR)/%.cc $(INCLUDES)
	$(CXX) $(CPPFLAGS) -c $< -o $@

$(BINDIR)/%: $(OBJDIR)/%.o $(OBJECTS)
	$(CXX) $(LDFLAGS) $^ $(LDLIBS) -o $@

