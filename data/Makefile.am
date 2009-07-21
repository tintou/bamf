servicedir=/usr/share/dbus-1/services
service_DATA=org.wncksync.Matcher.service

EXTRA_DIST=org.wncksync.Matcher.service.in

org.wncksync.Matcher.service: org.wncksync.Matcher.service.in
	sed "s_@LIBEXECDIR@_$(libexecdir)_" $< > $@
