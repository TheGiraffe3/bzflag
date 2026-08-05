lib_LTLIBRARIES = UGPlugin.la

UGPlugin_la_SOURCES = UGPlugin.cpp
UGPlugin_la_CPPFLAGS= -I$(top_srcdir)/include -I$(top_srcdir)/plugins/plugin_utils
UGPlugin_la_LDFLAGS = -module -avoid-version -shared
UGPlugin_la_LIBADD = $(top_builddir)/plugins/plugin_utils/libplugin_utils.la

AM_CPPFLAGS = $(CONF_CPPFLAGS)
AM_CFLAGS = $(CONF_CFLAGS)
AM_CXXFLAGS = $(CONF_CXXFLAGS)

MAINTAINERCLEANFILES =	\
	Makefile.in
