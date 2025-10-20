lib_LTLIBRARIES = SneakPlugin.la

SneakPlugin_la_SOURCES = SneakPlugin.cpp
SneakPlugin_la_CPPFLAGS= -I$(top_srcdir)/include -I$(top_srcdir)/plugins/plugin_utils
SneakPlugin_la_LDFLAGS = -module -avoid-version -shared
SneakPlugin_la_LIBADD = $(top_builddir)/plugins/plugin_utils/libplugin_utils.la

AM_CPPFLAGS = $(CONF_CPPFLAGS)
AM_CFLAGS = $(CONF_CFLAGS)
AM_CXXFLAGS = $(CONF_CXXFLAGS)

MAINTAINERCLEANFILES =	\
	Makefile.in
