EXTSOURCES =	xproto.py \
		bigreq.py \
		xc_misc.py
EXTENSION_XML = xproto.xml \
		bigreq.xml \
		xc_misc.xml

pkgpythondir = $(pythondir)/xcb

pkgpython_LTLIBRARIES = xcb.la

xcb_la_LIBADD = $(LIBXCB_LIBS)
xcb_la_CPPFLAGS = -I$(PYTHON_INCLUDE)
xcb_la_CFLAGS = -g $(CWARNFLAGS)
xcb_la_LDFLAGS = -module
xcb_la_SOURCES = conn.c cookie.c error.c event.c except.c \
		 ext.c extkey.c list.c module.c protobj.c \
		 reply.c request.c response.c struct.c union.c void.c

#pkgpython_PYTHON = xproto.py
#pkgpython_PYTHON += bigreq.py
#pkgpython_PYTHON += xc_misc.py

pkgpython_PYTHON = __init__.py
pkgpython_PYTHON += $(EXTSOURCES)

BUILT_SOURCES = $(EXTSOURCES)
CLEANFILES = $(EXTSOURCES) $(EXTENSION_XML)

$(EXTSOURCES): py_client.py

SUFFIXES = .xml

.xml.py:
	$(PYTHON) $(srcdir)/py_client.py -p $(XCBPROTO_XCBPYTHONDIR) $(XCBPROTO_XCBINCLUDEDIR)/$<

$(EXTENSION_XML):
	$(LN_S) -f $(XCBPROTO_XCBINCLUDEDIR)/$@ $@
