servicedir = @datarootdir@/dbus-1/services
service_DATA = org.ayatana.bamf.service

upstartdir = @datarootdir@/upstart/sessions
upstart_DATA = bamfdaemon.conf

org.ayatana.bamf.service: org.ayatana.bamf.service.in
	$(AM_V_GEN)sed "s,@PKGLIBEXECDIR@,$(pkglibexecdir)," $< > $@

bamfdaemon.conf: bamfdaemon.conf.in
	$(AM_V_GEN)sed "s,@PKGLIBEXECDIR@,$(pkglibexecdir)," $< > $@

EXTRA_DIST = org.ayatana.bamf.service.in \
             bamfdaemon.conf.in

CLEANFILES = org.ayatana.bamf.service \
             bamfdaemon.conf
