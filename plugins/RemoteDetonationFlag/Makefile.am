lib_LTLIBRARIES = RemoteDetonationFlag.la

RemoteDetonationFlag_la_SOURCES = RemoteDetonationFlag.cpp
RemoteDetonationFlag_la_CXXFLAGS= -I$(top_srcdir)/include -I$(top_srcdir)/plugins/plugin_utils
RemoteDetonationFlag_la_LDFLAGS = -module -avoid-version -shared
RemoteDetonationFlag_la_LIBADD = $(top_builddir)/plugins/plugin_utils/libplugin_utils.la

AM_CPPFLAGS = $(CONF_CPPFLAGS)
AM_CFLAGS = $(CONF_CFLAGS)
AM_CXXFLAGS = $(CONF_CXXFLAGS)

MAINTAINERCLEANFILES =	\
	Makefile.in
