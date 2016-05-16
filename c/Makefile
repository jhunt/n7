CFLAGS += -Wall

all: driver

driver: driver.o \
        frontend/reader.o
	$(CC) -o $@ $+

%.o: %.c pipeline.h
	$(CC) -c -o $@ $<

frontend/reader.o: frontend/reader.c pipeline.h
	$(CC) -c -o $@ $<
