lib_LTLIBRARIES = SeerRadar.la

SeerRadar_la_SOURCES = SeerRadar.cpp
SeerRadar_la_CPPFLAGS= -I$(top_srcdir)/include -I$(top_srcdir)/plugins/plugin_utils
SeerRadar_la_LDFLAGS = -module -avoid-version -shared
SeerRadar_la_LIBADD = $(top_builddir)/plugins/plugin_utils/libplugin_utils.la

AM_CPPFLAGS = $(CONF_CPPFLAGS)
AM_CFLAGS = $(CONF_CFLAGS)
AM_CXXFLAGS = $(CONF_CXXFLAGS)

MAINTAINERCLEANFILES =	\
	Makefile.in
