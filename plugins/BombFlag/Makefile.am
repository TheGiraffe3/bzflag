lib_LTLIBRARIES = BombFlag.la

BombFlag_la_SOURCES = BombFlag.cpp
BombFlag_la_LDFLAGS = -module -avoid-version -shared
BombFlag_la_CPPFLAGS= -I$(top_srcdir)/plugins/plugin_utils -I$(top_srcdir)/include
BombFlag_la_LIBADD  = $(top_builddir)/plugins/plugin_utils/libplugin_utils.la

AM_CPPFLAGS = $(CONF_CPPFLAGS)
AM_CFLAGS = $(CONF_CFLAGS)
AM_CXXFLAGS = $(CONF_CXXFLAGS)

MAINTAINERCLEANFILES =	\
	Makefile.in
