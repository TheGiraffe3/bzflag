lib_LTLIBRARIES = CascadeFlag.la

CascadeFlag_la_SOURCES = CascadeFlag.cpp
CascadeFlag_la_LDFLAGS = -module -avoid-version -shared

AM_CPPFLAGS = $(CONF_CPPFLAGS)
AM_CFLAGS = $(CONF_CFLAGS)
AM_CXXFLAGS = $(CONF_CXXFLAGS)

EXTRA_DIST = \
	CascadeFlag.sln \
	CascadeFlag.vcxproj	\
	CascadeFlag.vcxproj.filters

MAINTAINERCLEANFILES =	\
	Makefile.in
