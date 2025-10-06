lib_LTLIBRARIES = Kamikaze.la

Kamikaze_la_SOURCES = Kamikaze.cpp
Kamikaze_la_CPPFLAGS= -I$(top_srcdir)/include -I$(top_srcdir)/plugins/plugin_utils
Kamikaze_la_LDFLAGS = -module -avoid-version -shared
Kamikaze_la_LIBADD = $(top_builddir)/plugins/plugin_utils/libplugin_utils.la

AM_CPPFLAGS = $(CONF_CPPFLAGS)
AM_CFLAGS = $(CONF_CFLAGS)
AM_CXXFLAGS = $(CONF_CXXFLAGS)

EXTRA_DIST = \
	README.Kamikaze.txt \
	Kamikaze.sln \
	Kamikaze.vcxproj	\
	Kamikaze.vcxproj.filters

MAINTAINERCLEANFILES =	\
	Makefile.in
