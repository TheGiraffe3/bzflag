lib_LTLIBRARIES = VortexPlugin.la

VortexPlugin_la_SOURCES = VortexPlugin.cpp
VortexPlugin_la_CPPFLAGS= -I$(top_srcdir)/include -I$(top_srcdir)/plugins/plugin_utils
VortexPlugin_la_LDFLAGS = -module -avoid-version -shared
VortexPlugin_la_LIBADD = $(top_builddir)/plugins/plugin_utils/libplugin_utils.la

AM_CPPFLAGS = $(CONF_CPPFLAGS)
AM_CFLAGS = $(CONF_CFLAGS)
AM_CXXFLAGS = $(CONF_CXXFLAGS)

MAINTAINERCLEANFILES =	\
	Makefile.in
