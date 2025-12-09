lib_LTLIBRARIES = bzGunGameStyle.la

bzGunGameStyle_la_SOURCES = bzGunGameStyle.cpp
bzGunGameStyle_la_CPPFLAGS= -I$(top_srcdir)/include -I$(top_srcdir)/plugins/plugin_utils
bzGunGameStyle_la_LDFLAGS = -module -avoid-version -shared
bzGunGameStyle_la_LIBADD = $(top_builddir)/plugins/plugin_utils/libplugin_utils.la

AM_CPPFLAGS = $(CONF_CPPFLAGS)
AM_CFLAGS = $(CONF_CFLAGS)
AM_CXXFLAGS = $(CONF_CXXFLAGS)

MAINTAINERCLEANFILES =	\
	Makefile.in
