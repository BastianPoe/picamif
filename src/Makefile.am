bin_PROGRAMS = PiCamIf

PiCamIf_SOURCES = cam.h cam.c main.c http.h http.c strings.h strings.c list.h list.c

PiCamIf_LDFLAGS=-L/usr/local/lib
PiCamIf_LDADD=-lmicrohttpd -lgphoto2

AM_CFLAGS=-Wall -O2 $(INCLUDES) -g -I/usr/local/include
