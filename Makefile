###############################################################################
##  Makefile
##
##  Adapted from Public Scripts for use in mission-specific OSMs
##  Copyright (C) 2005-2011 Tom N Harris <telliamed@whoopdedo.org>
##  Copyright (C) 2012 Kevin Daughtridge <kevin@kdau.com>
##
##  This program is free software; you can redistribute it and/or modify
##  it under the terms of the GNU General Public License as published by
##  the Free Software Foundation; either version 2 of the License, or
##  (at your option) any later version.
##
##  This program is distributed in the hope that it will be useful,
##  but WITHOUT ANY WARRANTY; without even the implied warranty of
##  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
##  GNU General Public License for more details.
##
##  You should have received a copy of the GNU General Public License
##  along with this program. If not, see <http://www.gnu.org/licenses/>.
##
###############################################################################

.PHONY: default all clean
.PRECIOUS: %.o
.INTERMEDIATE: $(bindir)/exports.o
default: all

MODULE_NAME = CHANGEME_MODULE_NAME

srcdir = .
bindir = .

LGDIR = lg
SCRLIBDIR = ScriptLib
DH2DIR = DH2

PREFIX = i686-w64-mingw32-
CC = $(PREFIX)gcc
CXX = $(PREFIX)g++
AR = $(PREFIX)ar
LD = $(PREFIX)g++
DLLTOOL = $(PREFIX)dlltool
RC = $(PREFIX)windres

DEFINES = -DWINVER=0x0400 -D_WIN32_WINNT=0x0400 -DWIN32_LEAN_AND_MEAN \
	-D_DARKGAME=2 -D_NEWDARK
CXXDEFINES = -DMODULE_NAME=\"$(MODULE_NAME)\"
RCDEFINES = -DMODULE_NAME=\\\"$(MODULE_NAME)\\\"

INCLUDES = -I. -I$(srcdir) -I$(LGDIR) -I$(SCRLIBDIR) -I$(DH2DIR)
LIBDIRS = -L. -L$(LGDIR) -L$(SCRLIBDIR) -L$(DH2DIR)
LIBS = -ldh2 -luuid

# If you care for this... # -Wno-unused-variable
# A lot of the callbacks have unused parameters, so I turn that off.
CXXFLAGS = -W -Wall -masm=intel -std=gnu++0x
ARFLAGS = rc
LDFLAGS = -mwindows -mdll -Wl,--enable-auto-image-base -static-libgcc -static-libstdc++
DLLFLAGS = --add-underscore

ifdef DEBUG
DEFINES := $(DEFINES) -DDEBUG
CXXFLAGS := $(CXXFLAGS) -g -O0
LDFLAGS := $(LDFLAGS) -g
LIBS := -lScript2-d $(LIBS) -llg-d
else
DEFINES := $(DEFINES) -DNDEBUG
CXXFLAGS := $(CXXFLAGS) -O2
# LDFLAGS := $(LDFLAGS)
LIBS := -lScript2 $(LIBS) -llg
endif

MODULE_OBJS = \
	$(bindir)/Allocator.o \
	$(bindir)/Script.o \
	$(bindir)/ScriptDef.o \
	$(bindir)/ScriptModule.o \
	$(bindir)/exports.o \
	$(bindir)/script_res.o
$(bindir)/Allocator.o: Allocator.h
$(bindir)/Script.o: Script.h
$(bindir)/ScriptDef.o: custom.h BaseTrap.h BaseScript.h ScriptModule.h genscripts.h
$(bindir)/ScriptModule.o: ScriptModule.h Allocator.h
$(bindir)/script_res.o: script.rc version.rc

BASE_OBJS = \
	$(bindir)/BaseScript.o \
	$(bindir)/BaseTrap.o \
	$(bindir)/CommonScripts.o \
	$(bindir)/MsgHandlerArray.o \
	$(bindir)/utils.o
$(bindir)/BaseScript.o: BaseScript.h Script.h ScriptModule.h MsgHandlerArray.h
$(bindir)/BaseTrap.o: BaseTrap.h BaseScript.h Script.h
$(bindir)/CommonScripts.o: CommonScripts.h

CUSTOM_OBJS = \
	$(bindir)/custom.o
$(bindir)/custom.o: custom.h BaseScript.h Script.h scriptvars.h utils.h

$(bindir):
	mkdir -p $@

$(bindir)/%.o: $(srcdir)/%.cpp
	$(CXX) $(CXXFLAGS) $(DEFINES) $(CXXDEFINES) $(INCLUDES) -o $@ -c $<

$(bindir)/%_res.o: $(srcdir)/%.rc
	$(RC) $(DEFINES) $(RCDEFINES) -o $@ -i $<

$(bindir)/exports.o: $(bindir)/ScriptModule.o
	$(DLLTOOL) $(DLLFLAGS) --dllname $(MODULE_NAME).osm --output-exp $@ $^

$(MODULE_NAME).osm: $(CUSTOM_OBJS) $(BASE_OBJS) $(MODULE_OBJS)
	$(LD) $(LDFLAGS) -Wl,--image-base=0x11200000 $(LIBDIRS) -o $@ script.def $^ $(LIBS)

all: $(bindir) $(MODULE_NAME).osm

clean:
	$(RM) $(MODULE_NAME).osm $(bindir)/*.o

