include $(APPDIR)/Make.defs

PowerFeedDIR = $(APPDIR)/Machine/PowerFeed

# Application info
PROGNAME = powerfeed
PRIORITY = CONFIG_PowerFeed_PRIORITY
STACKSIZE = 4096

VPATH += $(PowerFeedDIR)/drivers
DEPPATH += --dep-path $(PowerFeedDIR)/drivers

OBJDIR = $(OUTDIR)/PowerFeed
OBJPATH = --obj-path $(OBJDIR) --obj-path $(OBJDIR)/drivers

# Include paths with full paths
CFLAGS += -I$(PowerFeedDIR)
CFLAGS += -I$(PowerFeedDIR)/drivers
CXXFLAGS += -I$(PowerFeedDIR)
CXXFLAGS += -I$(PowerFeedDIR)/drivers

# Source files with explicit paths
CSRCS = 
CXXSRCS = $(PowerFeedDIR)/Common.cxx \
          $(PowerFeedDIR)/Delay.cxx \
          $(PowerFeedDIR)/Display.cxx \
          $(PowerFeedDIR)/MachineState.cxx \
          $(PowerFeedDIR)/StepperState.cxx \
		  $(PowerFeedDIR)/drivers/ConsoleDisplay.cxx

MAINSRC = $(PowerFeedDIR)/main.cxx
MODULE = $(CONFIG_PowerFeed)

include $(APPDIR)/Application.mk