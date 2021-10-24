bin_PROGRAMS = pse2csv_for_d5a wtn2csv_for_d5a wth2csv_for_d5a

pse2csv_for_d5a_SOURCES = pse2csv_for_d5a_main.c pse2csv_for_d5a_print.h pse2csv_for_d5a_print.c
pse2csv_for_d5a_LDADD = ../lib/libalsep.a

wtn2csv_for_d5a_SOURCES = wtn2csv_for_d5a_main.c wtn2csv_for_d5a_print.h wtn2csv_for_d5a_print.c
wtn2csv_for_d5a_LDADD = ../lib/libalsep.a

wth2csv_for_d5a_SOURCES = wth2csv_for_d5a_main.c wth2csv_for_d5a_print.h wth2csv_for_d5a_print.c
wth2csv_for_d5a_LDADD = ../lib/libalsep.a

AM_CPPFLAGS = -I$(top_srcdir)/lib

EXTRA_DIST = pse2csv_for_d5a_print.h wtn2csv_for_d5a_print.h wth2csv_for_d5a_print.h
