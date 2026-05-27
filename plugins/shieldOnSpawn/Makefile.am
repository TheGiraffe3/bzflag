lib_LTLIBRARIES = shieldOnSpawn.la

shieldOnSpawn_la_SOURCES = shieldOnSpawn.cpp
shieldOnSpawn_la_CPPFLAGS= -I$(top_srcdir)/include -I$(top_srcdir)/plugins/plugin_utils
shieldOnSpawn_la_LDFLAGS = -module -avoid-version -shared
shieldOnSpawn_la_LIBADD = $(top_builddir)/plugins/plugin_utils/libplugin_utils.la

AM_CPPFLAGS = $(CONF_CPPFLAGS)
AM_CFLAGS = $(CONF_CFLAGS)
AM_CXXFLAGS = $(CONF_CXXFLAGS)

MAINTAINERCLEANFILES =	\
	Makefile.in
