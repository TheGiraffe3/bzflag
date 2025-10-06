lib_LTLIBRARIES = NoLossOnSW.la

NoLossOnSW_la_SOURCES = NoLossOnSW.cpp
NoLossOnSW_la_CPPFLAGS= -I$(top_srcdir)/include -I$(top_srcdir)/plugins/plugin_utils
NoLossOnSW_la_LDFLAGS = -module -avoid-version -shared
NoLossOnSW_la_LIBADD = $(top_builddir)/plugins/plugin_utils/libplugin_utils.la

AM_CPPFLAGS = $(CONF_CPPFLAGS)
AM_CFLAGS = $(CONF_CFLAGS)
AM_CXXFLAGS = $(CONF_CXXFLAGS)

EXTRA_DIST = \
	README.NoLossOnSW.txt \
	NoLossOnSW.sln \
	NoLossOnSW.vcxproj	\
	NoLossOnSW.vcxproj.filters

MAINTAINERCLEANFILES =	\
	Makefile.in
