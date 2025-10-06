lib_LTLIBRARIES = genoonce.la

genoonce_la_SOURCES = genoonce.cpp
genoonce_la_CPPFLAGS= -I$(top_srcdir)/include -I$(top_srcdir)/plugins/plugin_utils
genoonce_la_LDFLAGS = -module -avoid-version -shared
genoonce_la_LIBADD = $(top_builddir)/plugins/plugin_utils/libplugin_utils.la

AM_CPPFLAGS = $(CONF_CPPFLAGS)
AM_CFLAGS = $(CONF_CFLAGS)
AM_CXXFLAGS = $(CONF_CXXFLAGS)

EXTRA_DIST = \
	README.genoonce.txt \
	genoonce.sln \
	genoonce.vcxproj	\
	genoonce.vcxproj.filters

MAINTAINERCLEANFILES =	\
	Makefile.in
