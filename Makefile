###############################################################################
##  Makefile-gcc
##
##  This file is part of Public Scripts
##  Copyright (C) 2005-2011 Tom N Harris <telliamed@whoopdedo.org>
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


.SUFFIXES:
.SUFFIXES: .o .cpp .rc
.PRECIOUS: %.o

GAME = 2

srcdir = .
bindir = ./commonobj
bin1dir = ./t1obj
bin2dir = ./t2obj
bin3dir = ./ss2obj
bindirectories = $(bindir) $(bin1dir) $(bin2dir) $(bin3dir)

LGDIR = ../lg
SCRLIBDIR = ../ScriptLib
DH2DIR = ../DH2
DH2LIB = -ldh2

CC = gcc
CXX = g++
AR = ar
LD = g++
DLLTOOL = dlltool
RC = windres

DEFINES = -DWINVER=0x0400 -D_WIN32_WINNT=0x0400 -DWIN32_LEAN_AND_MEAN
GAME1 = -D_DARKGAME=1
GAME2 = -D_DARKGAME=2
GAME3 = -D_DARKGAME=3

ifdef DEBUG
DEFINES := $(DEFINES) -DDEBUG
CXXDEBUG = -g -O0
LDDEBUG = -g
LGLIB = -llg-d
SCR1LIB = -lScript1-d
SCR2LIB = -lScript2-d
SCR3LIB = -lScript3-d
else
DEFINES := $(DEFINES) -DNDEBUG
CXXDEBUG = -O2
LDDEBUG =
LGLIB = -llg
SCR1LIB = -lScript1
SCR2LIB = -lScript2
SCR3LIB = -lScript3
endif

ARFLAGS = rc
LDFLAGS = -mwindows -mdll -Wl,--enable-auto-image-base
LIBDIRS = -L. -L$(LGDIR) -L$(SCRLIBDIR) -L$(DH2DIR)
LIBS = $(DH2LIB) $(LGLIB) -luuid
INCLUDES = -I. -I$(srcdir) -I$(LGDIR) -I$(SCRLIBDIR) -I$(DH2DIR)
# If you care for this... # -Wno-unused-variable
# A lot of the callbacks have unused parameters, so I turn that off.
CXXFLAGS = -W -Wall -masm=intel
DLLFLAGS = --add-underscore

OSM_OBJS = $(bindir)/ScriptModule.o $(bindir)/Script.o $(bindir)/Allocator.o $(bindir)/exports.o
BASE_OBJS = $(bindir)/MsgHandlerArray.o
BASE1_OBJS = $(bin1dir)/BaseTrap.o $(bin1dir)/BaseScript.o $(bin1dir)/CommonScripts.o
BASE2_OBJS = $(bin2dir)/BaseTrap.o $(bin2dir)/BaseScript.o $(bin2dir)/CommonScripts.o
BASE3_OBJS = $(bin3dir)/BaseTrap.o $(bin3dir)/BaseScript.o $(bin3dir)/CommonScripts.o
SCR1_OBJS = $(bin1dir)/PublicScripts.o $(bin1dir)/T1Scripts.o
SCR2_OBJS = $(bin2dir)/PublicScripts.o $(bin2dir)/T2Scripts.o
SCR3_OBJS = $(bin3dir)/PublicScripts.o $(bin3dir)/SS2Scripts.o
MISC1_OBJS = $(bin1dir)/ScriptDef.o $(bin1dir)/utils.o
MISC2_OBJS = $(bin2dir)/ScriptDef.o $(bin2dir)/utils.o
MISC3_OBJS = $(bin3dir)/ScriptDef.o $(bin3dir)/utils.o
RES1_OBJS = $(bin1dir)/script_res.o
RES2_OBJS = $(bin2dir)/script_res.o
RES3_OBJS = $(bin3dir)/script_res.o


$(bindir)/%.o: $(srcdir)/%.cpp
	$(CXX) $(CXXFLAGS) $(CXXDEBUG) $(DEFINES) $(GAME2) $(INCLUDES) -o $@ -c $<

$(bin1dir)/%.o: $(srcdir)/%.cpp
	$(CXX) $(CXXFLAGS) $(CXXDEBUG) $(DEFINES) $(GAME1) $(INCLUDES) -o $@ -c $<

$(bin2dir)/%.o: $(srcdir)/%.cpp
	$(CXX) $(CXXFLAGS) $(CXXDEBUG) $(DEFINES) $(GAME2) $(INCLUDES) -o $@ -c $<

$(bin3dir)/%.o: $(srcdir)/%.cpp
	$(CXX) $(CXXFLAGS) $(CXXDEBUG) $(DEFINES) $(GAME3) $(INCLUDES) -o $@ -c $<

$(bindir)/%_res.o: $(srcdir)/%.rc
	$(RC) $(DEFINES) -o $@ -i $<

$(bin1dir)/%_res.o: $(srcdir)/%.rc
	$(RC) $(DEFINES) $(GAME1) -o $@ -i $<

$(bin2dir)/%_res.o: $(srcdir)/%.rc
	$(RC) $(DEFINES) $(GAME2) -o $@ -i $<

$(bin3dir)/%_res.o: $(srcdir)/%.rc
	$(RC) $(DEFINES) $(GAME3) -o $@ -i $<

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(CXXDEBUG) $(DEFINES) $(GAME2) $(INCLUDES) -o $@ -c $<

%_res.o: %.rc
	$(RC) $DEFINES) -o $@ -i $<

%.osm: %.o $(OSM_OBJS)
	$(LD) $(LDFLAGS) $(LDDEBUG) $(LIBDIRS) -o $@ script.def $< $(OSM_OBJS) $(SCR2LIB) $(LIBS)

all: $(bindirectories) script-t1.osm script-t2.osm script-ss2.osm version.osm

clean:
	$(RM) $(bindir)/* $(bin1dir)/* $(bin2dir)/* $(bin3dir)/*

$(bindir):
	mkdir -p $@
$(bin1dir):
	mkdir -p $@
$(bin2dir):
	mkdir -p $@
$(bin3dir):
	mkdir -p $@

#.INTERMEDIATE: $(bindir)/exports.o

$(bindir)/exports.o: $(bindir)/ScriptModule.o
	$(DLLTOOL) $(DLLFLAGS) --dllname script.osm --output-exp $@ $^

$(bindir)/ScriptModule.o: ScriptModule.cpp ScriptModule.h Allocator.h
$(bindir)/Script.o: Script.cpp Script.h
$(bindir)/Allocator.o: Allocator.cpp Allocator.h

$(bin1dir)/BaseScript.o: BaseScript.cpp BaseScript.h Script.h ScriptModule.h MsgHandlerArray.h
$(bin2dir)/BaseScript.o: BaseScript.cpp BaseScript.h Script.h ScriptModule.h MsgHandlerArray.h
$(bin3dir)/BaseScript.o: BaseScript.cpp BaseScript.h Script.h ScriptModule.h MsgHandlerArray.h
$(bin1dir)/BaseTrap.o: BaseTrap.cpp BaseTrap.h BaseScript.h Script.h
$(bin2dir)/BaseTrap.o: BaseTrap.cpp BaseTrap.h BaseScript.h Script.h
$(bin3dir)/BaseTrap.o: BaseTrap.cpp BaseTrap.h BaseScript.h Script.h

$(bin1dir)/CommonScripts.o: CommonScripts.cpp CommonScripts.h
$(bin2dir)/CommonScripts.o: CommonScripts.cpp CommonScripts.h
$(bin3dir)/CommonScripts.o: CommonScripts.cpp CommonScripts.h

$(bin1dir)/PublicScripts.o: PublicScripts.cpp PublicScripts.h CommonScripts.h BaseTrap.h BaseScript.h Script.h
$(bin2dir)/PublicScripts.o: PublicScripts.cpp PublicScripts.h CommonScripts.h BaseTrap.h BaseScript.h Script.h
$(bin3dir)/PublicScripts.o: PublicScripts.cpp PublicScripts.h CommonScripts.h BaseTrap.h BaseScript.h Script.h

$(bin1dir)/T1Scripts.o: T1Scripts.cpp T1Scripts.h CommonScripts.h BaseTrap.h BaseScript.h Script.h
$(bin2dir)/T2Scripts.o: T2Scripts.cpp T2Scripts.h CommonScripts.h BaseTrap.h BaseScript.h Script.h
$(bin3dir)/SS2Scripts.o: SS2Scripts.cpp SS2Scripts.h CommonScripts.h BaseTrap.h BaseScript.h Script.h

$(bin1dir)/ScriptDef.o: ScriptDef.cpp PublicScripts.h T1Scripts.h BaseTrap.h BaseScript.h ScriptModule.h genscripts.h
$(bin2dir)/ScriptDef.o: ScriptDef.cpp PublicScripts.h T2Scripts.h BaseTrap.h BaseScript.h ScriptModule.h genscripts.h
$(bin3dir)/ScriptDef.o: ScriptDef.cpp PublicScripts.h SS2Scripts.h BaseTrap.h BaseScript.h ScriptModule.h genscripts.h

$(bin1dir)/script_res.o: script.rc version.rc
$(bin2dir)/script_res.o: script.rc version.rc
$(bin3dir)/script_res.o: script.rc version.rc

script-t1.osm: $(SCR1_OBJS) $(BASE1_OBJS) $(BASE_OBJS) $(OSM_OBJS) $(MISC1_OBJS) $(RES1_OBJS)
	$(LD) $(LDFLAGS) -Wl,--image-base=0x11200000 $(LDDEBUG) $(LIBDIRS) -o $@ script.def $^ $(SCR1LIB) $(LIBS)

script-t2.osm: $(SCR2_OBJS) $(BASE2_OBJS) $(BASE_OBJS) $(OSM_OBJS) $(MISC2_OBJS) $(RES2_OBJS)
	$(LD) $(LDFLAGS) -Wl,--image-base=0x11200000 $(LDDEBUG) $(LIBDIRS) -o $@ script.def $^ $(SCR2LIB) $(LIBS)

script-ss2.osm: $(SCR3_OBJS) $(BASE3_OBJS) $(BASE_OBJS) $(OSM_OBJS) $(MISC3_OBJS) $(RES3_OBJS)
	$(LD) $(LDFLAGS) -Wl,--image-base=0x11200000 $(LDDEBUG) $(LIBDIRS) -o $@ script.def $^ $(SCR3LIB) $(LIBS)

$(bindir)/scrversion.o: scrversion.cpp scrversion.h
	$(CXX) $(CXXFLAGS) $(CXXDEBUG) $(DEFINES) $(GAME3) $(INCLUDES) -o $@ -c $<

version.osm: $(bindir)/scrversion.o $(OSM_OBJS) $(bindir)/scrversion_res.o
	$(LD) $(LDFLAGS) -Wl,--image-base=0x12100000 $(LDDEBUG) $(LIBDIRS) -o $@ script.def $^ $(SCR2LIB) $(LIBS) -lversion
