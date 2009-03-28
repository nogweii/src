/^[	 ]*#[	 ]*undef/!b
t clr
: clr
s,^\([	 ]*\)#\([	 ]*\)undef\([	 ][	 ]*\)PACKAGE_NAME$,\1#\2define\3PACKAGE_NAME "",;t
s,^\([	 ]*\)#\([	 ]*\)undef\([	 ][	 ]*\)PACKAGE_TARNAME$,\1#\2define\3PACKAGE_TARNAME "",;t
s,^\([	 ]*\)#\([	 ]*\)undef\([	 ][	 ]*\)PACKAGE_VERSION$,\1#\2define\3PACKAGE_VERSION "",;t
s,^\([	 ]*\)#\([	 ]*\)undef\([	 ][	 ]*\)PACKAGE_STRING$,\1#\2define\3PACKAGE_STRING "",;t
s,^\([	 ]*\)#\([	 ]*\)undef\([	 ][	 ]*\)PACKAGE_BUGREPORT$,\1#\2define\3PACKAGE_BUGREPORT "",;t
s,^\([	 ]*\)#\([	 ]*\)undef\([	 ][	 ]*\)PACKAGE$,\1#\2define\3PACKAGE "pwman",;t
s,^\([	 ]*\)#\([	 ]*\)undef\([	 ][	 ]*\)VERSION$,\1#\2define\3VERSION "0.3.9",;t
s,^\([	 ]*\)#\([	 ]*\)undef\([	 ][	 ]*\)STDC_HEADERS$,\1#\2define\3STDC_HEADERS 1,;t
s,^\([	 ]*\)#\([	 ]*\)undef\([	 ][	 ]*\)HAVE_SYS_TYPES_H$,\1#\2define\3HAVE_SYS_TYPES_H 1,;t
s,^\([	 ]*\)#\([	 ]*\)undef\([	 ][	 ]*\)HAVE_SYS_STAT_H$,\1#\2define\3HAVE_SYS_STAT_H 1,;t
s,^\([	 ]*\)#\([	 ]*\)undef\([	 ][	 ]*\)HAVE_STDLIB_H$,\1#\2define\3HAVE_STDLIB_H 1,;t
s,^\([	 ]*\)#\([	 ]*\)undef\([	 ][	 ]*\)HAVE_STRING_H$,\1#\2define\3HAVE_STRING_H 1,;t
s,^\([	 ]*\)#\([	 ]*\)undef\([	 ][	 ]*\)HAVE_MEMORY_H$,\1#\2define\3HAVE_MEMORY_H 1,;t
s,^\([	 ]*\)#\([	 ]*\)undef\([	 ][	 ]*\)HAVE_STRINGS_H$,\1#\2define\3HAVE_STRINGS_H 1,;t
s,^\([	 ]*\)#\([	 ]*\)undef\([	 ][	 ]*\)HAVE_INTTYPES_H$,\1#\2define\3HAVE_INTTYPES_H 1,;t
s,^\([	 ]*\)#\([	 ]*\)undef\([	 ][	 ]*\)HAVE_STDINT_H$,\1#\2define\3HAVE_STDINT_H 1,;t
s,^\([	 ]*\)#\([	 ]*\)undef\([	 ][	 ]*\)HAVE_UNISTD_H$,\1#\2define\3HAVE_UNISTD_H 1,;t
s,^\([	 ]*\)#\([	 ]*\)undef\([	 ][	 ]*\)HAVE_LOCALE_H$,\1#\2define\3HAVE_LOCALE_H 1,;t
s,^\([	 ]*\)#\([	 ]*\)undef\([	 ][	 ]*\)HAVE_TERMIOS_H$,\1#\2define\3HAVE_TERMIOS_H 1,;t
s,^\([	 ]*\)#\([	 ]*\)undef\([	 ][	 ]*\)HAVE_LINUX_TERMIOS_H$,\1#\2define\3HAVE_LINUX_TERMIOS_H 1,;t
s,^\([	 ]*\)#\([	 ]*\)undef\([	 ][	 ]*\)HAVE_SYS_IOCTL_H$,\1#\2define\3HAVE_SYS_IOCTL_H 1,;t
s,^\([	 ]*\)#\([	 ]*\)undef\([	 ][	 ]*\)HAVE_STRING_H$,\1#\2define\3HAVE_STRING_H 1,;t
s,^\([	 ]*\)#\([	 ]*\)undef\([	 ][	 ]*\)HAVE_DRAND48$,\1#\2define\3HAVE_DRAND48 1,;t
s,^\([	 ]*\)#\([	 ]*\)undef\([	 ][	 ]*\)HAVE_NCURSES_H$,\1#\2define\3HAVE_NCURSES_H 1,;t
s,^\([	 ]*\)#\([	 ]*\)undef\([	 ][	 ]*\)STDC_HEADERS$,\1#\2define\3STDC_HEADERS 1,;t
s,^\([	 ]*\)#\([	 ]*\)undef\([	 ][	 ]*\)HAVE_STRING_H$,\1#\2define\3HAVE_STRING_H 1,;t
s,^[	 ]*#[	 ]*undef[	 ][	 ]*[a-zA-Z_][a-zA-Z_0-9]*,/* & */,
