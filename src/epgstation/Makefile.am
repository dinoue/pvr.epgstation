noinst_LTLIBRARIES = libepgstation.la

libepgstation_la_SOURCES = \
	../base64/base64.cpp \
	api.cpp \
	channels.cpp \
	recorded.cpp \
	reserves.cpp \
	rules.cpp \
	schedule.cpp \
	storage.cpp \
	docs.cpp

libepgstation_la_LDFLAGS = \
	-avoid-version

AM_CPPFLAGS = \
	$(AM_CPPFLAGS_EXT) \
	-std=c++11 \
	-Wall \
	-I. \
	-I$(top_srcdir)/include -I$(top_srcdir)/src

EXTRA_DIST = \
	include

all: libepgstation.la
