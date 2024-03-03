TEMPLATE = subdirs

SUBDIRS= ../../src \
         rtknavi_qt \
         rtkget_qt \
         rtkplot_qt \
         rtkpost_qt \
         rtklaunch_qt \
         srctblbrows_qt \
         strsvr_qt \
         rtkconv_qt



app.depends = ../../src

rtknavi_qt.depends = ../../src
rtkget_qt.depends = ../../src
rtkplot_qt.depends = ../../src
rtkpost_qt.depends = ../../src
rtklaunch_qt.depends = ../../src
rtkconv_qt.depends = ../../src
srctblbrows_qt.depends = ../../src
strsvr_qt.depends = ../../src

IERS_MODEL {
    SUBDIRS += lib
    app.depend = lib
}
