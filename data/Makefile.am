servicedir = @datarootdir@/dbus-1/services
service_DATA = org.ayatana.bamf.service

upstartdir = @datarootdir@/upstart/sessions
upstart_DATA = bamfdaemon.conf

bamfdaemondbusrunnerdir = $(pkglibexecdir)
bamfdaemondbusrunner_SCRIPTS = bamfdaemon-dbus-runner

org.ayatana.bamf.service: org.ayatana.bamf.service.in
	$(AM_V_GEN)sed "s,@PKGLIBEXECDIR@,$(pkglibexecdir)," $< > $@

bamfdaemon.conf: bamfdaemon.conf.in
	$(AM_V_GEN)sed "s,@PKGLIBEXECDIR@,$(pkglibexecdir)," $< > $@

bamfdaemon-dbus-runner: bamfdaemon-dbus-runner.in
	$(AM_V_GEN)sed "s,@PKGLIBEXECDIR@,$(pkglibexecdir)," $< > $@

EXTRA_DIST = org.ayatana.bamf.service.in \
             bamfdaemon.conf.in \
             bamfdaemon-dbus-runner.in

CLEANFILES = org.ayatana.bamf.service \
             bamfdaemon.conf \
             bamfdaemon-dbus-runner
