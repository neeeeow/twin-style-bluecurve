AUTOMAKE_OPTIONS = foreign

SUBDIRS =

KDE_CXXFLAGS = -DQT_PLUGIN

INCLUDES = $(all_includes) -I$(kde_includes)/kwin

kwindir = $(kde_datadir)/kwin/
kwin_DATA = bluecurve.desktop

noinst_HEADERS =  bluecurveclient.h bitmaps.h

kde_module_LTLIBRARIES = kwin3_bluecurve.la
kwin3_bluecurve_la_SOURCES = bluecurveclient.cpp
kwin3_bluecurve_la_LIBADD = $(kde_libraries)/libkdecorations.la
kwin3_bluecurve_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN) -lkdecore -module
kwin3_bluecurve_la_METASOURCES = AUTO

DISTCLEANFILES = $(kwin3_bluecurve_la_METASOURCES)

EXTRA_DIST=bluecurveclient.moc bluecurve.desktop
