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

top_srcdir = .

SLICE_SRCS = RTAReceiver.ice RTAMonitor.ice RTACommand.ice
OBJS       = RTAReceiver.o RTAMonitor.o RTACommand.o

CLIENT     = RTAEBSim
COBJS      = $(CLIENT).o
SRCS       = $(OBJS:.o=.cpp) $(COBJS: .o=.cpp)

TARGETS    = $(CLIENT)

include $(top_srcdir)/config/Make.rules

CPPFLAGS := -I. $(CPPFLAGS) $(EXTRA_CPPFLAGS)

LIBS       += -l RTAtelem -l packet

$(CLIENT): $(OBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(COBJS) $(LIBS)
