all: pqos heracles

pqos:
	cd lib/pqos && make
	cd lib && mkdir -p so
	cd lib/pqos && cp libpqos.so.1.2.0 ../so/libpqos.so

heracles:
	cd src && make

.PHONY: clean

clean:
	cd lib/pqos && make clean
	cd src && make clean
	