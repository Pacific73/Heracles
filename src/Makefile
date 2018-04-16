CXX = g++

LIBDIR ?= ../lib/headers

CXXFLAGS = -g -Wall -O2 -std=c++11 -I$(LIBDIR)

LDFLAGS = -pthread

TARGET = heracles

OBJECTS = main.o top_controller.o core_memory_controller.o \
		  tap.o info_puller.o \
		  db_driver.o cpu_driver.o memory_driver.o cache_driver.o

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(OBJECTS) -o ../bin/heracles
	@echo [INFO] Compilation accomplished.

main.o: main.cpp heracles.h
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ -c $< 

top_controller.o: top_controller.cpp top_controller.h
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ -c $<

core_memory_controller.o: core_memory_controller.cpp core_memory_controller.h
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ -c $<

info_puller.o: info_puller.cpp info_puller.h
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ -c $<

tap.o: tap.cpp tap.h
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ -c $<

db_driver.o: db_driver.cpp db_driver.h
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ -c $<

cpu_driver.o: cpu_driver.cpp cpu_driver.h
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ -c $<

cache_driver.o: cache_driver.cpp cache_driver.h
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ -c $<

memory_driver.o: memory_driver.cpp memory_driver.h
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ -c $<

.PHONY: clean

clean:
	@-rm *.o
	@-rm ../bin/heracles
	@echo [INFO] Clean accomplished.


