#
# OMNeT++/OMNEST Makefile for Lab1
#
# This file was generated with the command:
#  opp_makemake -f --deep -O out
#

# Name of target to be created (-o option)
TARGET = Lab1$(EXE_SUFFIX)

# User interface (uncomment one) (-u option)
USERIF_LIBS = $(ALL_ENV_LIBS) # that is, $(TKENV_LIBS) $(CMDENV_LIBS)
#USERIF_LIBS = $(CMDENV_LIBS)
#USERIF_LIBS = $(TKENV_LIBS)

# C++ include paths (with -I)
INCLUDE_PATH = -I. -Isrc -Isrc/Terminal -Isrc/Tools -Isrc/Tools/HDLC

# Additional object and library files to link with
EXTRA_OBJS =

# Additional libraries (-L, -l options)
LIBS =

# Output directory
PROJECT_OUTPUT_DIR = out
PROJECTRELATIVE_PATH =
O = $(PROJECT_OUTPUT_DIR)/$(CONFIGNAME)/$(PROJECTRELATIVE_PATH)

# Object files for local .cpp and .msg files
OBJS = \
    $O/src/Terminal/Application.o \
    $O/src/Terminal/Link.o \
    $O/src/Terminal/Middle.o \
    $O/src/Tools/MovingWindow.o \
    $O/src/Tools/HDLC/Control.o \
    $O/src/Tools/HDLC/Frame.o

# Message files
MSGFILES =

#------------------------------------------------------------------------------

# Pull in OMNeT++ configuration (Makefile.inc or configuser.vc)

ifneq ("$(OMNETPP_CONFIGFILE)","")
CONFIGFILE = $(OMNETPP_CONFIGFILE)
else
ifneq ("$(OMNETPP_ROOT)","")
CONFIGFILE = $(OMNETPP_ROOT)/Makefile.inc
else
CONFIGFILE = $(shell opp_configfilepath)
endif
endif

ifeq ("$(wildcard $(CONFIGFILE))","")
$(error Config file '$(CONFIGFILE)' does not exist -- add the OMNeT++ bin directory to the path so that opp_configfilepath can be found, or set the OMNETPP_CONFIGFILE variable to point to Makefile.inc)
endif

include $(CONFIGFILE)

# Simulation kernel and user interface libraries
OMNETPP_LIB_SUBDIR = $(OMNETPP_LIB_DIR)/$(TOOLCHAIN_NAME)
OMNETPP_LIBS = -L"$(OMNETPP_LIB_SUBDIR)" -L"$(OMNETPP_LIB_DIR)" -loppmain$D $(USERIF_LIBS) $(KERNEL_LIBS) $(SYS_LIBS)

COPTS = $(CFLAGS)  $(INCLUDE_PATH) -I$(OMNETPP_INCL_DIR)
MSGCOPTS = $(INCLUDE_PATH)

# we want to recompile everything if COPTS changes,
# so we store COPTS into $COPTS_FILE and have object
# files depend on it (except when "make depend" was called)
COPTS_FILE = $O/.last-copts
ifneq ($(MAKECMDGOALS),depend)
ifneq ("$(COPTS)","$(shell cat $(COPTS_FILE) 2>/dev/null || echo '')")
$(shell $(MKPATH) "$O" && echo "$(COPTS)" >$(COPTS_FILE))
endif
endif

#------------------------------------------------------------------------------
# User-supplied makefile fragment(s)
# >>>
# <<<
#------------------------------------------------------------------------------

# Main target
all: $O/$(TARGET)
	$(Q)$(LN) $O/$(TARGET) .

$O/$(TARGET): $(OBJS)  $(wildcard $(EXTRA_OBJS)) Makefile
	@$(MKPATH) $O
	@echo Creating executable: $@
	$(Q)$(CXX) $(LDFLAGS) -o $O/$(TARGET)  $(OBJS) $(EXTRA_OBJS) $(AS_NEEDED_OFF) $(WHOLE_ARCHIVE_ON) $(LIBS) $(WHOLE_ARCHIVE_OFF) $(OMNETPP_LIBS)

.PHONY: all clean cleanall depend msgheaders

.SUFFIXES: .cpp

$O/%.o: %.cpp $(COPTS_FILE)
	@$(MKPATH) $(dir $@)
	$(qecho) "$<"
	$(Q)$(CXX) -c $(COPTS) -o $@ $<

%_m.cpp %_m.h: %.msg
	$(qecho) MSGC: $<
	$(Q)$(MSGC) -s _m.cpp $(MSGCOPTS) $?

msgheaders: $(MSGFILES:.msg=_m.h)

clean:
	$(qecho) Cleaning...
	$(Q)-rm -rf $O
	$(Q)-rm -f Lab1 Lab1.exe libLab1.so libLab1.a libLab1.dll libLab1.dylib
	$(Q)-rm -f ./*_m.cpp ./*_m.h
	$(Q)-rm -f src/*_m.cpp src/*_m.h
	$(Q)-rm -f src/Terminal/*_m.cpp src/Terminal/*_m.h
	$(Q)-rm -f src/Tools/*_m.cpp src/Tools/*_m.h
	$(Q)-rm -f src/Tools/HDLC/*_m.cpp src/Tools/HDLC/*_m.h

cleanall: clean
	$(Q)-rm -rf $(PROJECT_OUTPUT_DIR)

depend:
	$(qecho) Creating dependencies...
	$(Q)$(MAKEDEPEND) $(INCLUDE_PATH) -f Makefile -P\$$O/ -- $(MSG_CC_FILES)  ./*.cpp src/*.cpp src/Terminal/*.cpp src/Tools/*.cpp src/Tools/HDLC/*.cpp

# DO NOT DELETE THIS LINE -- make depend depends on it.
$O/src/Terminal/Application.o: src/Terminal/Application.cpp \
	src/Terminal/Application.h
$O/src/Terminal/Link.o: src/Terminal/Link.cpp \
	src/Terminal/Link.h \
	src/Tools/HDLC/Control.h \
	src/Tools/HDLC/Frame.h \
	src/Tools/MovingWindow.h
$O/src/Terminal/Middle.o: src/Terminal/Middle.cpp \
	src/Terminal/Middle.h
$O/src/Tools/MovingWindow.o: src/Tools/MovingWindow.cpp \
	src/Tools/HDLC/Control.h \
	src/Tools/HDLC/Frame.h \
	src/Tools/MovingWindow.h
$O/src/Tools/HDLC/Control.o: src/Tools/HDLC/Control.cpp \
	src/Tools/HDLC/Control.h
$O/src/Tools/HDLC/Frame.o: src/Tools/HDLC/Frame.cpp \
	src/Tools/HDLC/Control.h \
	src/Tools/HDLC/Frame.h

