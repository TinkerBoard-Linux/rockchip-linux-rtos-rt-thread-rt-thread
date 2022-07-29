#
# Copyright (C) 2001-2006 by egnite Software GmbH.
# Copyright (C) 2008 by egnite GmbH.
#
# All rights reserved.
#
# For additional information see http://www.ethernut.de/
#
# $Id: Makefile,v 1.1 2008/10/05 17:01:15 haraldkipp Exp $
#

PROJ = libhxmp3

top_srcdir = ../..
top_blddir = ../..

CFLAGS=-I./
#-D__CW32__
SRCS = main.c \
       mp3dec.c \
       mp3tabs.c \
       buffers.c \
       dequant.c \
       hufftabs.c  \
       polyphase.c \
       subband.c \
       dqchan.c  \
       imdct.c   \
       scalfact.c \
       bitstream.c  \
       dct32.c   \
       huffman.c \
       stproc.c \
       trigtabs.c

OBJS = $(SRCS:.c=.o)

TARG = libhxmp3.a
TARG_BIN = main
#.a

all: $(TARG)
    $(CC) $(OBJS) -o $(TARG_BIN) -I./

$(TARG): $(OBJS)
    $(AR) $(ARFLAGS)  $(TARG) $(OBJS)

install: $(TARG)
    $(CP) $(TARG) $(LIBDIR)/$(TARG)
ifeq ($(ARCH), unix)
    $(RANLIB) $(LIBDIR)/$(TARG)
endif

clean:
    @echo "  [CLEANING] $(TARG)..."
    @-rm -f $(OBJS)
    #@-rm -f $(TARG)
    @-rm -f $(SRCS:.c=.lst)
    @-rm -f $(SRCS:.c=.lis)
    #@-rm -f $(SRCS:.c=.s)
    @-rm -f $(SRCS:.c=.bak)
    @-rm -f $(SRCS:.c=.c~)
    @-rm -f $(SRCS:.c=.i)
    @-rm -f $(TARG_BIN)
