lib_LTLIBRARIES = SMinePlugin.la

SMinePlugin_la_SOURCES = SMinePlugin.cpp
SMinePlugin_la_CPPFLAGS= -I$(top_srcdir)/include -I$(top_srcdir)/plugins/plugin_utils
SMinePlugin_la_LDFLAGS = -module -avoid-version -shared
SMinePlugin_la_LIBADD = $(top_builddir)/plugins/plugin_utils/libplugin_utils.la

AM_CPPFLAGS = $(CONF_CPPFLAGS)
AM_CFLAGS = $(CONF_CFLAGS)
AM_CXXFLAGS = $(CONF_CXXFLAGS)

MAINTAINERCLEANFILES =	\
	Makefile.in
