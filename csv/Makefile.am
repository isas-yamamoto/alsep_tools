bin_PROGRAMS = pse2csv wtn2csv wth2csv

pse2csv_SOURCES = pse2csv.c csv.c csv.h
pse2csv_LDADD = ../lib/libalsep.a

wtn2csv_SOURCES = wtn2csv.c csv.c csv.h
wtn2csv_LDADD = ../lib/libalsep.a

wth2csv_SOURCES = wth2csv.c csv.c csv.h
wth2csv_LDADD = ../lib/libalsep.a

AM_CPPFLAGS = -I$(top_srcdir)/lib

EXTRA_DIST = csv.h
