lib_LTLIBRARIES = SHPlugin.la

SHPlugin_la_SOURCES = SHPlugin.cpp
SHPlugin_la_CPPFLAGS= -I$(top_srcdir)/include -I$(top_srcdir)/plugins/plugin_utils
SHPlugin_la_LDFLAGS = -module -avoid-version -shared
SHPlugin_la_LIBADD = $(top_builddir)/plugins/plugin_utils/libplugin_utils.la

AM_CPPFLAGS = $(CONF_CPPFLAGS)
AM_CFLAGS = $(CONF_CFLAGS)
AM_CXXFLAGS = $(CONF_CXXFLAGS)

MAINTAINERCLEANFILES =	\
	Makefile.in
