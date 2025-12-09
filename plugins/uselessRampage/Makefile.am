lib_LTLIBRARIES = uselessRampage.la

uselessRampage_la_SOURCES = uselessRampage.cpp
uselessRampage_la_CXXFLAGS= -I$(top_srcdir)/include -I$(top_srcdir)/plugins/plugin_utils
uselessRampage_la_LDFLAGS = -module -avoid-version -shared
uselessRampage_la_LIBADD = $(top_builddir)/plugins/plugin_utils/libplugin_utils.la

AM_CPPFLAGS = $(CONF_CPPFLAGS)
AM_CFLAGS = $(CONF_CFLAGS)
AM_CXXFLAGS = $(CONF_CXXFLAGS)

EXTRA_DIST = \
	README.txt \
	uselessRampage.def \
	uselessRampage.sln \
	uselessRampage.vcxproj

MAINTAINERCLEANFILES =	\
	Makefile.in
