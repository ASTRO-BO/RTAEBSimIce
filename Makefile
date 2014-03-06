# **********************************************************************
#
# Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir = .

SLICE_SRCS = RTAReceiver.ice
OBJS       = RTAReceiver.o

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
