CXX = g++

LIBDIR ?= ../lib/headers

NET_LIBDIR = -I/usr/lib/llvm-3.7/include -I/usr/include/bcc

LDDIR ?= ../lib/so

CXXFLAGS = -g -Wall -O2 -std=c++11 -I$(LIBDIR) -L$(LDDIR)

LDFLAGS = -lpthread -lsqlite3 -lpqos -lbcc -lbpf

TARGET = heracles

OBJECTS = main.o top_controller.o core_memory_controller.o \
		  tap.o info_puller.o helpers.o \
		  db_driver.o cpu_driver.o memory_driver.o cache_driver.o \
		  network_controller.o network_monitor.o network_driver.o

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(OBJECTS) -o ../bin/heracles $(LDFLAGS)
	@echo [INFO] Compilation accomplished.

main.o: main.cpp heracles.h
	$(CXX) $(CXXFLAGS) $(NET_LIBDIR) -o $@ -c $<

top_controller.o: top_controller.cpp top_controller.h
	$(CXX) $(CXXFLAGS) -o $@ -c $<

core_memory_controller.o: core_memory_controller.cpp core_memory_controller.h
	$(CXX) $(CXXFLAGS) -o $@ -c $<

info_puller.o: info_puller.cpp info_puller.h
	$(CXX) $(CXXFLAGS) -o $@ -c $<

tap.o: tap.cpp tap.h
	$(CXX) $(CXXFLAGS) -o $@ -c $<

db_driver.o: db_driver.cpp db_driver.h
	$(CXX) $(CXXFLAGS) -o $@ -c $<

cpu_driver.o: cpu_driver.cpp cpu_driver.h
	$(CXX) $(CXXFLAGS) -o $@ -c $<

cache_driver.o: cache_driver.cpp cache_driver.h
	$(CXX) $(CXXFLAGS) -o $@ -c $<

memory_driver.o: memory_driver.cpp memory_driver.h
	$(CXX) $(CXXFLAGS) -o $@ -c $<

network_controller.o: network_controller.cpp network_controller.h
	$(CXX) $(CXXFLAGS) $(NET_LIBDIR) -o $@ -c $<

network_monitor.o: network_monitor.cpp network_monitor.h
	$(CXX) $(CXXFLAGS) $(NET_LIBDIR) -o $@ -c $<

network_driver.o: network_driver.cpp network_driver.h
	$(CXX) $(CXXFLAGS) $(NET_LIBDIR) -o $@ -c $<

helpers.o: helpers.cpp helpers.h
	$(CXX) $(CXXFLAGS) -o $@ -c $<

.PHONY: clean

clean:
	@-rm *.o
	@-rm ../bin/heracles
	@echo [INFO] Clean accomplished.


