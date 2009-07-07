
EXTRA_DIST =  

# Warning: This is an automatically generated file, do not edit!

if ENABLE_DEBUG
ASSEMBLY_COMPILER_COMMAND = gmcs
ASSEMBLY_COMPILER_FLAGS =  -noconfig -codepage:utf8 -warn:4 -optimize- -debug "-define:DEBUG"
ASSEMBLY = bin/Debug/WnckSyncDaemon.exe
ASSEMBLY_MDB = $(ASSEMBLY).mdb
COMPILE_TARGET = exe
PROJECT_REFERENCES = 
BUILD_DIR = bin/Debug

WNCKSYNCDAEMON_EXE_MDB_SOURCE=bin/Debug/WnckSyncDaemon.exe.mdb
WNCKSYNCDAEMON_EXE_MDB=$(BUILD_DIR)/WnckSyncDaemon.exe.mdb

endif

if ENABLE_RELEASE
ASSEMBLY_COMPILER_COMMAND = gmcs
ASSEMBLY_COMPILER_FLAGS =  -noconfig -codepage:utf8 -warn:4 -optimize-
ASSEMBLY = bin/Release/WnckSyncDaemon.exe
ASSEMBLY_MDB = 
COMPILE_TARGET = exe
PROJECT_REFERENCES = 
BUILD_DIR = bin/Release

WNCKSYNCDAEMON_EXE_MDB=

endif

AL=al2
SATELLITE_ASSEMBLY_NAME=$(notdir $(basename $(ASSEMBLY))).resources.dll

PROGRAMFILES = \
	$(WNCKSYNCDAEMON_EXE_MDB)  

BINARIES = \
	$(WNCKSYNCDAEMON)  


RESGEN=resgen2
	
all: $(ASSEMBLY) $(PROGRAMFILES) $(BINARIES) 

FILES = \
	src/WnckSyncDaemon/WnckSyncDaemon.cs \
	src/WnckSyncDaemon/IControl.cs \
	src/WnckSyncDaemon/Control.cs \
	src/WnckSyncDaemon/WindowMatcher.cs 

DATA_FILES = 

RESOURCES = 

EXTRAS = \
	src \
	src/WnckSyncDaemon \
	wncksyncdaemon.in 

REFERENCES =  \
	System \
	System.Core \
	$(WNCK_SHARP_10_LIBS) \
	$(GNOME_DESKTOP_SHARP_20_LIBS) \
	$(NDESK_DBUS_10_LIBS) \
	$(NDESK_DBUS_GLIB_10_LIBS) \
	$(GTK_SHARP_20_LIBS)

DLL_REFERENCES = 

CLEANFILES = $(PROGRAMFILES) $(BINARIES) 

include $(top_srcdir)/Makefile.include

WNCKSYNCDAEMON = $(BUILD_DIR)/wncksyncdaemon

$(eval $(call emit-deploy-wrapper,WNCKSYNCDAEMON,wncksyncdaemon,x))


$(eval $(call emit_resgen_targets))
$(build_xamlg_list): %.xaml.g.cs: %.xaml
	xamlg '$<'

$(ASSEMBLY) $(ASSEMBLY_MDB): $(build_sources) $(build_resources) $(build_datafiles) $(DLL_REFERENCES) $(PROJECT_REFERENCES) $(build_xamlg_list) $(build_satellite_assembly_list)
	mkdir -p $(shell dirname $(ASSEMBLY))
	$(ASSEMBLY_COMPILER_COMMAND) $(ASSEMBLY_COMPILER_FLAGS) -out:$(ASSEMBLY) -target:$(COMPILE_TARGET) $(build_sources_embed) $(build_resources_embed) $(build_references_ref)
