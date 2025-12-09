lib_LTLIBRARIES = neutralZones.la

neutralZones_la_SOURCES = neutralZones.cpp
neutralZones_la_CPPFLAGS= -I$(top_srcdir)/include -I$(top_srcdir)/plugins/plugin_utils
neutralZones_la_LDFLAGS = -module -avoid-version -shared
neutralZones_la_LIBADD = $(top_builddir)/plugins/plugin_utils/libplugin_utils.la

AM_CPPFLAGS = $(CONF_CPPFLAGS)
AM_CFLAGS = $(CONF_CFLAGS)
AM_CXXFLAGS = $(CONF_CXXFLAGS)

EXTRA_DIST = \
	LICENSE.md \
	README.md \
	neutralZones.def \
	neutralZones.sln \
	neutralZones.vcxproj \
	neutralZones.vcxproj.filters

MAINTAINERCLEANFILES =	\
	Makefile.in
