lib_LTLIBRARIES = mappedDeathZones.la

mappedDeathZones_la_SOURCES = mappedDeathZones.cpp
mappedDeathZones_la_CPPFLAGS= -I$(top_srcdir)/include -I$(top_srcdir)/plugins/plugin_utils
mappedDeathZones_la_LDFLAGS = -module -avoid-version -shared
mappedDeathZones_la_LIBADD = $(top_builddir)/plugins/plugin_utils/libplugin_utils.la

AM_CPPFLAGS = $(CONF_CPPFLAGS)
AM_CFLAGS = $(CONF_CFLAGS)
AM_CXXFLAGS = $(CONF_CXXFLAGS)

EXTRA_DIST = \
	LICENSE.md \
	README.md \
	mappedDeathZones.def \
	mappedDeathZones.sln \
	mappedDeathZones.vcxproj \
	mappedDeathZones.vcxproj.filters

MAINTAINERCLEANFILES =	\
	Makefile.in
