lib_LTLIBRARIES = doublePoint.la

doublePoint_la_SOURCES = doublePoint.cpp
doublePoint_la_CPPFLAGS= -I$(top_srcdir)/include -I$(top_srcdir)/plugins/plugin_utils
doublePoint_la_LDFLAGS = -module -avoid-version -shared
doublePoint_la_LIBADD = $(top_builddir)/plugins/plugin_utils/libplugin_utils.la

AM_CPPFLAGS = $(CONF_CPPFLAGS)
AM_CFLAGS = $(CONF_CFLAGS)
AM_CXXFLAGS = $(CONF_CXXFLAGS)

EXTRA_DIST = \
	README.doublePoint.txt \
	doublePoint.sln \
	doublePoint.vcxproj	\
	doublePoint.vcxproj.filters

MAINTAINERCLEANFILES =	\
	Makefile.in
