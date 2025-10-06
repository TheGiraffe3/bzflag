lib_LTLIBRARIES = genostats.la

genostats_la_SOURCES = genostats.cpp
genostats_la_CPPFLAGS= -I$(top_srcdir)/include -I$(top_srcdir)/plugins/plugin_utils
genostats_la_LDFLAGS = -module -avoid-version -shared
genostats_la_LIBADD = $(top_builddir)/plugins/plugin_utils/libplugin_utils.la

AM_CPPFLAGS = $(CONF_CPPFLAGS)
AM_CFLAGS = $(CONF_CFLAGS)
AM_CXXFLAGS = $(CONF_CXXFLAGS)

EXTRA_DIST = \
	README.genostats.txt \
	genostats.sln \
	genostats.vcxproj	\
	genostats.vcxproj.filters

MAINTAINERCLEANFILES =	\
	Makefile.in
