:t
  /@[a-zA-Z_][a-zA-Z_0-9]*@/!b
s,@SHELL@,/bin/sh,;t t
s,@PATH_SEPARATOR@,:,;t t
s,@PACKAGE_NAME@,,;t t
s,@PACKAGE_TARNAME@,,;t t
s,@PACKAGE_VERSION@,,;t t
s,@PACKAGE_STRING@,,;t t
s,@PACKAGE_BUGREPORT@,,;t t
s,@exec_prefix@,${prefix},;t t
s,@prefix@,/home/colin/sys,;t t
s,@program_transform_name@,s\,x\,x\,,;t t
s,@bindir@,${exec_prefix}/bin,;t t
s,@sbindir@,${exec_prefix}/sbin,;t t
s,@libexecdir@,${exec_prefix}/libexec,;t t
s,@datadir@,${prefix}/share,;t t
s,@sysconfdir@,${prefix}/etc,;t t
s,@sharedstatedir@,${prefix}/com,;t t
s,@localstatedir@,${prefix}/var,;t t
s,@libdir@,${exec_prefix}/lib,;t t
s,@includedir@,${prefix}/include,;t t
s,@oldincludedir@,/usr/include,;t t
s,@infodir@,${prefix}/info,;t t
s,@mandir@,${prefix}/man,;t t
s,@build_alias@,,;t t
s,@host_alias@,,;t t
s,@target_alias@,,;t t
s,@DEFS@,-DHAVE_CONFIG_H,;t t
s,@ECHO_C@,,;t t
s,@ECHO_N@,-n,;t t
s,@ECHO_T@,,;t t
s,@LIBS@, -lcurses -lxml2 -lz -lm,;t t
s,@INSTALL_PROGRAM@,${INSTALL},;t t
s,@INSTALL_SCRIPT@,${INSTALL},;t t
s,@INSTALL_DATA@,${INSTALL} -m 644,;t t
s,@CYGPATH_W@,echo,;t t
s,@PACKAGE@,pwman,;t t
s,@VERSION@,0.3.9,;t t
s,@ACLOCAL@,aclocal-1.9,;t t
s,@AUTOCONF@,autoconf,;t t
s,@AUTOMAKE@,automake-1.9,;t t
s,@AUTOHEADER@,autoheader,;t t
s,@MAKEINFO@,makeinfo,;t t
s,@install_sh@,/home/colin/sys/src/pwman/install-sh,;t t
s,@STRIP@,,;t t
s,@ac_ct_STRIP@,,;t t
s,@INSTALL_STRIP_PROGRAM@,${SHELL} $(install_sh) -c -s,;t t
s,@mkdir_p@,mkdir -p --,;t t
s,@AWK@,gawk,;t t
s,@SET_MAKE@,,;t t
