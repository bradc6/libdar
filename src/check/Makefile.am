
#
# conditionnal part: --enable-mode=...
#

localedir = @localedir@
system_path = @sysconfdir@

if BUILD_MODE32
    MYLIB=dar32
    AM_CPPFLAGS=-DLIBDAR_MODE=32 -I../libdar -DDAR_LOCALEDIR=\"$(localedir)\" -DDAR_SYS_DIR=\"$(system_path)\" $(CPP_PROF)
else
if BUILD_MODE64
    MYLIB=dar64
    AM_CPPFLAGS=-DLIBDAR_MODE=64 -I../libdar -DDAR_LOCALEDIR=\"$(localedir)\" -DDAR_SYS_DIR=\"$(system_path)\" $(CPP_PROF)
else
    MYLIB=dar
    AM_CPPFLAGS=-I../libdar -DDAR_LOCALEDIR=\"$(localedir)\"  -DDAR_SYS_DIR=\"$(system_path)\" $(CPP_PROF)
endif
endif

dist_noinst_DATA = build_tree.sh main.sh modif_tree.sh routine.sh loop.sh
noinst_PROGRAMS = all_features
check_PROGRAMS = permission
LDADD = -L../libdar -l$(MYLIB)
AM_LDFLAGS = @LTLIBINTL@

permission:
	chmod a+x $(dist_noinst_DATA) loop.sh
	./loop.sh

all_features_SOURCES = all_features.cpp
all_features_DEPENDENCIES = ../libdar/lib$(MYLIB).la
permission_SOURCES =

clean-local:
	chmod u+x $(dist_noinst_DATA)
	OPT=tmp.file DAR=none ./routine.sh NONE
	rm -f tmp.file


