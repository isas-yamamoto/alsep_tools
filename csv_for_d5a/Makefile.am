bin_PROGRAMS = pse2csv_for_d5a wtn2csv_for_d5a wth2csv_for_d5a

pse2csv_for_d5a_SOURCES = pse2csv_for_d5a.c csv.c csv.h
pse2csv_for_d5a_LDADD = ../lib/libalsep.a

wtn2csv_for_d5a_SOURCES = wtn2csv_for_d5a.c csv.c csv.h
wtn2csv_for_d5a_LDADD = ../lib/libalsep.a

wth2csv_for_d5a_SOURCES = wth2csv_for_d5a.c csv.c csv.h
wth2csv_for_d5a_LDADD = ../lib/libalsep.a

AM_CPPFLAGS = -I$(top_srcdir)/lib

EXTRA_DIST = csv.h
