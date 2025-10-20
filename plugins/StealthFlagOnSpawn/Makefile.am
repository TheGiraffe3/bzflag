lib_LTLIBRARIES = StealthFlagOnSpawn.la

StealthFlagOnSpawn_la_SOURCES = StealthFlagOnSpawn.cpp
StealthFlagOnSpawn_la_CPPFLAGS= -I$(top_srcdir)/include -I$(top_srcdir)/plugins/plugin_utils
StealthFlagOnSpawn_la_LDFLAGS = -module -avoid-version -shared
StealthFlagOnSpawn_la_LIBADD = $(top_builddir)/plugins/plugin_utils/libplugin_utils.la

AM_CPPFLAGS = $(CONF_CPPFLAGS)
AM_CFLAGS = $(CONF_CFLAGS)
AM_CXXFLAGS = $(CONF_CXXFLAGS)

MAINTAINERCLEANFILES =	\
	Makefile.in
