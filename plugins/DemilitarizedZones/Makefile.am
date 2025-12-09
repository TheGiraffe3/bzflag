lib_LTLIBRARIES = DemilitarizedZones.la

DemilitarizedZones_la_SOURCES = DemilitarizedZones.cpp
DemilitarizedZones_la_CPPFLAGS= -I$(top_srcdir)/include -I$(top_srcdir)/plugins/plugin_utils
DemilitarizedZones_la_LDFLAGS = -module -avoid-version -shared
DemilitarizedZones_la_LIBADD = $(top_builddir)/plugins/plugin_utils/libplugin_utils.la

AM_CPPFLAGS = $(CONF_CPPFLAGS)
AM_CFLAGS = $(CONF_CFLAGS)
AM_CXXFLAGS = $(CONF_CXXFLAGS)

EXTRA_DIST = \
	LICENSE.md \
	README.md \
	DemilitarizedZones.def \
	DemilitarizedZones.sln \
	DemilitarizedZones.vcxproj \
	DemilitarizedZones.vcxproj.filters

MAINTAINERCLEANFILES =	\
	Makefile.in
