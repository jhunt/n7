CFLAGS += -Wall

all: driver

driver: driver.o \
        frontend/reader.o
	$(CC) $(LDFLAGS) -o $@ $+

%.o: %.c pipeline.h
	$(CC) $(CPPFLAGS) -c $(CFLAGS) -o $@ $<

frontend/reader.o: frontend/reader.c pipeline.h
	$(CC) $(CPPFLAGS) -c $(CFLAGS) -o $@ $<
