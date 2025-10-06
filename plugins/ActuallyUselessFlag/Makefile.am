lib_LTLIBRARIES = ActuallyUselessFlag.la

ActuallyUselessFlag_la_SOURCES = ActuallyUselessFlag.cpp
ActuallyUselessFlag_la_CPPFLAGS= -I$(top_srcdir)/include -I$(top_srcdir)/plugins/plugin_utils
ActuallyUselessFlag_la_LDFLAGS = -module -avoid-version -shared
ActuallyUselessFlag_la_LIBADD = $(top_builddir)/plugins/plugin_utils/libplugin_utils.la

AM_CPPFLAGS = $(CONF_CPPFLAGS)
AM_CFLAGS = $(CONF_CFLAGS)
AM_CXXFLAGS = $(CONF_CXXFLAGS)

MAINTAINERCLEANFILES =	\
	Makefile.in
