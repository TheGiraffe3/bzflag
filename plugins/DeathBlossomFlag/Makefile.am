lib_LTLIBRARIES = DeathBlossomFlag.la

DeathBlossomFlag_la_SOURCES = DeathBlossomFlag.cpp
DeathBlossomFlag_la_CPPFLAGS= -I$(top_srcdir)/include -I$(top_srcdir)/plugins/plugin_utils
DeathBlossomFlag_la_LDFLAGS = -module -avoid-version -shared
DeathBlossomFlag_la_LIBADD = $(top_builddir)/plugins/plugin_utils/libplugin_utils.la

AM_CPPFLAGS = $(CONF_CPPFLAGS)
AM_CFLAGS = $(CONF_CFLAGS)
AM_CXXFLAGS = $(CONF_CXXFLAGS)

EXTRA_DIST = \
	README.DeathBlossomFlag.txt \
	DeathBlossomFlag.sln \
	DeathBlossomFlag.vcxproj	\
	DeathBlossomFlag.vcxproj.filters

MAINTAINERCLEANFILES =	\
	Makefile.in
