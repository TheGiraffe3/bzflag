lib_LTLIBRARIES = uselessGeno.la

uselessGeno_la_SOURCES = uselessGeno.cpp
uselessGeno_la_CPPFLAGS= -I$(top_srcdir)/include -I$(top_srcdir)/plugins/plugin_utils
uselessGeno_la_LDFLAGS = -module -avoid-version -shared
uselessGeno_la_LIBADD = $(top_builddir)/plugins/plugin_utils/libplugin_utils.la

AM_CPPFLAGS = $(CONF_CPPFLAGS)
AM_CFLAGS = $(CONF_CFLAGS)
AM_CXXFLAGS = $(CONF_CXXFLAGS)

EXTRA_DIST = \
	README.uselessGeno.txt \
	uselessGeno.sln \
	uselessGeno.vcxproj	\
	uselessGeno.vcxproj.filters

MAINTAINERCLEANFILES =	\
	Makefile.in
