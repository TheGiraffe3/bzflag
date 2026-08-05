lib_LTLIBRARIES = GunGametankatek.la

GunGametankatek_la_SOURCES = GunGametankatek.cpp
GunGametankatek_la_CPPFLAGS= -I$(top_srcdir)/include -I$(top_srcdir)/plugins/plugin_utils
GunGametankatek_la_LDFLAGS = -module -avoid-version -shared
GunGametankatek_la_LIBADD = $(top_builddir)/plugins/plugin_utils/libplugin_utils.la

AM_CPPFLAGS = $(CONF_CPPFLAGS)
AM_CFLAGS = $(CONF_CFLAGS)
AM_CXXFLAGS = $(CONF_CXXFLAGS)

MAINTAINERCLEANFILES =	\
	Makefile.in
