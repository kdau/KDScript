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
bindir = ./commonobj
bin1dir = ./t1obj
bin2dir = ./t2obj
bin3dir = ./ss2obj
bindirectories = $(bindir) $(bin1dir) $(bin2dir) $(bin3dir)

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

DEFINES = -DWINVER=0x0400 -D_WIN32_WINNT=0x0400 -DWIN32_LEAN_AND_MEAN
CXXDEFINES = -DMODULE_NAME=\"$(MODULE_NAME)\"
RCDEFINES = -DMODULE_NAME=\\\"$(MODULE_NAME)\\\"
GAME1 = -D_DARKGAME=1
GAME2 = -D_DARKGAME=2
GAME3 = -D_DARKGAME=3

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
LIBS := $(LIBS) -llg-d
SCR1LIB = -lScript1-d
SCR2LIB = -lScript2-d
SCR3LIB = -lScript3-d
else
DEFINES := $(DEFINES) -DNDEBUG
CXXFLAGS := $(CXXFLAGS) -O2
# LDFLAGS := $(LDFLAGS)
LIBS := $(LIBS) -llg
SCR1LIB = -lScript1
SCR2LIB = -lScript2
SCR3LIB = -lScript3
endif

MODULE_OBJS = $(bindir)/ScriptModule.o $(bindir)/Script.o $(bindir)/Allocator.o $(bindir)/exports.o
MODULE1_OBJS = $(bin1dir)/ScriptDef.o $(bin1dir)/script_res.o
MODULE2_OBJS = $(bin2dir)/ScriptDef.o $(bin2dir)/script_res.o
MODULE3_OBJS = $(bin3dir)/ScriptDef.o $(bin3dir)/script_res.o

BASE_OBJS = $(bindir)/MsgHandlerArray.o
BASE1_OBJS = $(bin1dir)/BaseTrap.o $(bin1dir)/BaseScript.o $(bin1dir)/CommonScripts.o $(bin1dir)/utils.o
BASE2_OBJS = $(bin2dir)/BaseTrap.o $(bin2dir)/BaseScript.o $(bin2dir)/CommonScripts.o $(bin2dir)/utils.o
BASE3_OBJS = $(bin3dir)/BaseTrap.o $(bin3dir)/BaseScript.o $(bin3dir)/CommonScripts.o $(bin3dir)/utils.o

CUSTOM1_OBJS = $(bin1dir)/custom.o
CUSTOM2_OBJS = $(bin2dir)/custom.o
CUSTOM3_OBJS = $(bin3dir)/custom.o

$(bindir):
	mkdir -p $@
$(bin1dir):
	mkdir -p $@
$(bin2dir):
	mkdir -p $@
$(bin3dir):
	mkdir -p $@

$(bindir)/%.o: $(srcdir)/%.cpp
	$(CXX) $(CXXFLAGS) $(DEFINES) $(CXXDEFINES) $(INCLUDES) -o $@ -c $<
$(bin1dir)/%.o: $(srcdir)/%.cpp
	$(CXX) $(CXXFLAGS) $(DEFINES) $(CXXDEFINES) $(GAME1) $(INCLUDES) -o $@ -c $<
$(bin2dir)/%.o: $(srcdir)/%.cpp
	$(CXX) $(CXXFLAGS) $(DEFINES) $(CXXDEFINES) $(GAME2) $(INCLUDES) -o $@ -c $<
$(bin3dir)/%.o: $(srcdir)/%.cpp
	$(CXX) $(CXXFLAGS) $(DEFINES) $(CXXDEFINES) $(GAME3) $(INCLUDES) -o $@ -c $<

$(bindir)/%_res.o: $(srcdir)/%.rc
	$(RC) $(DEFINES) $(RCDEFINES) -o $@ -i $<
$(bin1dir)/%_res.o: $(srcdir)/%.rc
	$(RC) $(DEFINES) $(RCDEFINES) $(GAME1) -o $@ -i $<
$(bin2dir)/%_res.o: $(srcdir)/%.rc
	$(RC) $(DEFINES) $(RCDEFINES) $(GAME2) -o $@ -i $<
$(bin3dir)/%_res.o: $(srcdir)/%.rc
	$(RC) $(DEFINES) $(RCDEFINES) $(GAME3) -o $@ -i $<

$(bindir)/exports.o: $(bindir)/ScriptModule.o
	$(DLLTOOL) $(DLLFLAGS) --dllname $(MODULE_NAME).osm --output-exp $@ $^

$(bindir)/ScriptModule.o: ScriptModule.h Allocator.h
$(bindir)/Script.o: Script.h
$(bindir)/Allocator.o: Allocator.h

$(bin1dir)/BaseScript.o: BaseScript.h Script.h ScriptModule.h MsgHandlerArray.h
$(bin2dir)/BaseScript.o: BaseScript.h Script.h ScriptModule.h MsgHandlerArray.h
$(bin3dir)/BaseScript.o: BaseScript.h Script.h ScriptModule.h MsgHandlerArray.h

$(bin1dir)/BaseTrap.o: BaseTrap.h BaseScript.h Script.h
$(bin2dir)/BaseTrap.o: BaseTrap.h BaseScript.h Script.h
$(bin3dir)/BaseTrap.o: BaseTrap.h BaseScript.h Script.h

$(bin1dir)/CommonScripts.o: CommonScripts.h
$(bin2dir)/CommonScripts.o: CommonScripts.h
$(bin3dir)/CommonScripts.o: CommonScripts.h

$(bin1dir)/custom.o: custom.h BaseScript.h Script.h scriptvars.h utils.h
$(bin2dir)/custom.o: custom.h BaseScript.h Script.h scriptvars.h utils.h
$(bin3dir)/custom.o: custom.h BaseScript.h Script.h scriptvars.h utils.h

$(bin1dir)/ScriptDef.o: custom.h BaseTrap.h BaseScript.h ScriptModule.h genscripts.h
$(bin2dir)/ScriptDef.o: custom.h BaseTrap.h BaseScript.h ScriptModule.h genscripts.h
$(bin3dir)/ScriptDef.o: custom.h BaseTrap.h BaseScript.h ScriptModule.h genscripts.h

$(bin1dir)/script_res.o: script.rc version.rc
$(bin2dir)/script_res.o: script.rc version.rc
$(bin3dir)/script_res.o: script.rc version.rc

$(MODULE_NAME)-t1.osm: $(CUSTOM1_OBJS) $(BASE1_OBJS) $(BASE_OBJS) $(MODULE_OBJS) $(MODULE1_OBJS)
	$(LD) $(LDFLAGS) -Wl,--image-base=0x11200000 $(LIBDIRS) -o $@ script.def $^ $(SCR1LIB) $(LIBS)

$(MODULE_NAME)-t2.osm: $(CUSTOM2_OBJS) $(BASE2_OBJS) $(BASE_OBJS) $(MODULE_OBJS) $(MODULE2_OBJS)
	$(LD) $(LDFLAGS) -Wl,--image-base=0x11200000 $(LIBDIRS) -o $@ script.def $^ $(SCR2LIB) $(LIBS)

$(MODULE_NAME)-ss2.osm: $(CUSTOM3_OBJS) $(BASE3_OBJS) $(BASE_OBJS) $(MODULE_OBJS) $(MODULE3_OBJS)
	$(LD) $(LDFLAGS) -Wl,--image-base=0x11200000 $(LIBDIRS) -o $@ script.def $^ $(SCR3LIB) $(LIBS)

all: $(bindirectories) $(MODULE_NAME)-t1.osm $(MODULE_NAME)-t2.osm $(MODULE_NAME)-ss2.osm

clean:
	$(RM) $(MODULE_NAME)-*.osm $(bindir)/*.o $(bin1dir)/*.o $(bin2dir)/*.o $(bin3dir)/*.o

