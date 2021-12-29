BUILT_SOURCES = parser.h
AM_YFLAGS = -d

lib_LTLIBRARIES = libinputparser.la
libinputparser_la_SOURCES = parser.y lexer.l
libinputparser_la_CPPFLAGS = -I$(top_srcdir)/include

# TODO: make this a static library
