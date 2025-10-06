lib_LTLIBRARIES = riskygeno.la

riskygeno_la_SOURCES = riskygeno.cpp
riskygeno_la_CPPFLAGS= -I$(top_srcdir)/include -I$(top_srcdir)/plugins/plugin_utils
riskygeno_la_LDFLAGS = -module -avoid-version -shared
riskygeno_la_LIBADD = $(top_builddir)/plugins/plugin_utils/libplugin_utils.la

AM_CPPFLAGS = $(CONF_CPPFLAGS)
AM_CFLAGS = $(CONF_CFLAGS)
AM_CXXFLAGS = $(CONF_CXXFLAGS)

EXTRA_DIST = \
	README.riskygeno.txt \
	riskygeno.sln \
	riskygeno.vcxproj	\
	riskygeno.vcxproj.filters

MAINTAINERCLEANFILES =	\
	Makefile.in
