lib_LTLIBRARIES = SpawnWithRandomFlag.la

SpawnWithRandomFlag_la_SOURCES = SpawnWithRandomFlag.cpp
SpawnWithRandomFlag_la_CPPFLAGS= -I$(top_srcdir)/include -I$(top_srcdir)/plugins/plugin_utils
SpawnWithRandomFlag_la_LDFLAGS = -module -avoid-version -shared
SpawnWithRandomFlag_la_LIBADD = $(top_builddir)/plugins/plugin_utils/libplugin_utils.la

AM_CPPFLAGS = $(CONF_CPPFLAGS)
AM_CFLAGS = $(CONF_CFLAGS)
AM_CXXFLAGS = $(CONF_CXXFLAGS)

MAINTAINERCLEANFILES =	\
	Makefile.in
