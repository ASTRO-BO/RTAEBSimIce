#**************************************************************************#
#   begin                : Mar 06 2014                                     #
#   copyright            : (C) 2014 Andrea Zoli                            #
#   email                : zoli@iasfbo.inaf.it                             #
#**************************************************************************#

#**************************************************************************#
#                                                                          #
# This program is free software; you can redistribute it and/or modify     #
# it under the terms of the GNU General Public License as published by     #
# the Free Software Foundation; either version 2 of the License, or        #
# (at your option) any later version.                                      #
#                                                                          #
#**************************************************************************#

# Prefix for each installed program. Only ABSOLUTE PATH
prefix=/usr/local
exec_prefix=$(prefix)
# The directory to install the binary files in.
bindir=$(exec_prefix)/bin
# The directory to install the libraries in.
libdir=$(exec_prefix)/lib
# The directory to install the include files in.
includedir=$(exec_prefix)/include
# The directory to install the local configuration file.
datadir=$(exec_prefix)/share

top_srcdir = .

SLICE_SRCS = RTAReceiver.ice RTAMonitor.ice RTACommand.ice
OBJS       = RTAReceiver.o RTAMonitor.o RTACommand.o

CLIENT     = RTAEBSim
COBJS      = $(CLIENT).o
SRCS       = $(OBJS:.o=.cpp) $(COBJS: .o=.cpp)

TARGETS    = $(CLIENT)

include $(top_srcdir)/config/Make.rules

CPPFLAGS ?= -O2
CPPFLAGS += -I.

LIBS += -l RTAtelem -l packet

$(CLIENT): $(OBJS) $(COBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(COBJS) $(LIBS)

install:
	test -d $(bindir) || mkdir -p $(bindir)
	cp -pf RTAEBSim $(bindir)
	test -d $(datadir)/ebsim || mkdir -p $(datadir)/ebsim
	cp -pf config.sim $(datadir)/ebsim
