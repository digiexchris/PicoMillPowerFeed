include $(APPDIR)/Make.defs

PICOMILLDIR = $(APPDIR)/Machine/PicoMill

# Application info
PROGNAME = picomill
PRIORITY = SCHED_PRIORITY_DEFAULT
STACKSIZE = 2048

VPATH += $(PICOMILLDIR)/drivers
DEPPATH += --dep-path $(PICOMILLDIR)/drivers

OBJDIR = $(OUTDIR)/picomill
OBJPATH = --obj-path $(OBJDIR) --obj-path $(OBJDIR)/drivers

# Include paths with full paths
CFLAGS += -I$(PICOMILLDIR)
CFLAGS += -I$(PICOMILLDIR)/drivers
CXXFLAGS += -I$(PICOMILLDIR)
CXXFLAGS += -I$(PICOMILLDIR)/drivers

# Source files with explicit paths
CSRCS = 
CXXSRCS = $(PICOMILLDIR)/Common.cxx \
          $(PICOMILLDIR)/Delay.cxx \
          $(PICOMILLDIR)/Display.cxx \
          $(PICOMILLDIR)/MachineState.cxx \
          $(PICOMILLDIR)/StepperState.cxx \
		  $(PICOMILLDIR)/drivers/PicoSSD1306Display.cxx

MAINSRC = $(PICOMILLDIR)/picomill_main.cxx
MODULE = $(CONFIG_PICOMILL)

include $(APPDIR)/Application.mk