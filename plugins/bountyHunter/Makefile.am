lib_LTLIBRARIES = bountyHunter.la

bountyHunter_la_SOURCES = bountyHunter.cpp
bountyHunter_la_CXXFLAGS= -I$(top_srcdir)/include -I$(top_srcdir)/plugins/plugin_utils
bountyHunter_la_LDFLAGS = -module -avoid-version -shared
bountyHunter_la_LIBADD = $(top_builddir)/plugins/plugin_utils/libplugin_utils.la

AM_CPPFLAGS = $(CONF_CPPFLAGS)
AM_CFLAGS = $(CONF_CFLAGS)
AM_CXXFLAGS = $(CONF_CXXFLAGS)

EXTRA_DIST = \
	README.txt \
	bountyHunter.def \
	bountyHunter.sln \
	bountyHunter.vcxproj

MAINTAINERCLEANFILES =	\
	Makefile.in
