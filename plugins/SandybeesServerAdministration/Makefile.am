lib_LTLIBRARIES = SandybeesServerAdministration.la

SandybeesServerAdministration_la_SOURCES = SandybeesServerAdministration.cpp
SandybeesServerAdministration_la_CPPFLAGS= -I$(top_srcdir)/include -I$(top_srcdir)/plugins/plugin_utils
SandybeesServerAdministration_la_LDFLAGS = -module -avoid-version -shared
SandybeesServerAdministration_la_LIBADD = $(top_builddir)/plugins/plugin_utils/libplugin_utils.la

AM_CPPFLAGS = $(CONF_CPPFLAGS)
AM_CFLAGS = $(CONF_CFLAGS)
AM_CXXFLAGS = $(CONF_CXXFLAGS)

MAINTAINERCLEANFILES =	\
	Makefile.in
