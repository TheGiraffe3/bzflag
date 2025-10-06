lib_LTLIBRARIES = superLaserFlag.la

superLaserFlag_la_SOURCES = superLaserFlag.cpp
superLaserFlag_la_LDFLAGS = -module -avoid-version -shared
superLaserFlag_la_CPPFLAGS= -I$(top_srcdir)/plugins/plugin_utils -I$(top_srcdir)/include
superLaserFlag_la_LIBADD  = $(top_builddir)/plugins/plugin_utils/libplugin_utils.la

AM_CPPFLAGS = $(CONF_CPPFLAGS)
AM_CFLAGS = $(CONF_CFLAGS)
AM_CXXFLAGS = $(CONF_CXXFLAGS)

MAINTAINERCLEANFILES =	\
	Makefile.in
