lib_LTLIBRARIES = spawnFacingCenter.la

spawnFacingCenter_la_SOURCES = spawnFacingCenter.cpp
spawnFacingCenter_la_CPPFLAGS= -I$(top_srcdir)/include -I$(top_srcdir)/plugins/plugin_utils
spawnFacingCenter_la_LDFLAGS = -module -avoid-version -shared
spawnFacingCenter_la_LIBADD = $(top_builddir)/plugins/plugin_utils/libplugin_utils.la

AM_CPPFLAGS = $(CONF_CPPFLAGS)
AM_CFLAGS = $(CONF_CFLAGS)
AM_CXXFLAGS = $(CONF_CXXFLAGS)

EXTRA_DIST = \
	README.spawnFacingCenter.txt \
	spawnFacingCenter.sln \
	spawnFacingCenter.vcxproj	\
	spawnFacingCenter.vcxproj.filters

MAINTAINERCLEANFILES =	\
	Makefile.in
