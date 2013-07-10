###############################################################################
##  Makefile
##
##  Copyright (C) 2012-2013 Kevin Daughtridge <kevin@kdau.com>
##  Adapted in part from Public Scripts
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

bindir = ./bin-all
bin1dir = ./bin-t1
bin2dir = ./bin-t2

.PHONY: default all clean
.PRECIOUS: %.o
.INTERMEDIATE: $(bindir)/exports.o
default: all

MODULE_NAME = KDScript

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

DEFINES = -DWINVER=0x0400 -D_WIN32_WINNT=0x0400 -DWIN32_LEAN_AND_MEAN -D_NEWDARK
DEFINES1 = -D_DARKGAME=1
DEFINES2 = -D_DARKGAME=2
CXXDEFINES = -DMODULE_NAME=\"$(MODULE_NAME)\"
RCDEFINES = -DMODULE_NAME=\\\"$(MODULE_NAME)\\\"

INCLUDES = -I. -I$(LGDIR) -I$(SCRLIBDIR) -I$(DH2DIR)
LIBDIRS = -L. -L$(LGDIR) -L$(SCRLIBDIR) -L$(DH2DIR)
LIBS = -ldh2 -luuid

# If you care for this... # -Wno-unused-variable
# A lot of the callbacks have unused parameters, so I turn that off.
CXXFLAGS = -W -Wall -masm=intel -std=gnu++0x
ARFLAGS = rc
LDFLAGS = -mwindows -mdll -Wl,--enable-auto-image-base -static-libgcc -static-libstdc++
DLLFLAGS = --add-underscore

DEBUG = DEBUG #FIXME
ifdef DEBUG
DEFINES := $(DEFINES) -DDEBUG
CXXFLAGS := $(CXXFLAGS) -g -O0
LDFLAGS := $(LDFLAGS) -g
LIBS := $(LIBS) -llg-d
LIBS1 := $(LIBS1) -lScript1-d
LIBS2 := $(LIBS2) -lScript2-d
else
DEFINES := $(DEFINES) -DNDEBUG
CXXFLAGS := $(CXXFLAGS) -O2
# LDFLAGS := $(LDFLAGS)
LIBS := $(LIBS) -llg
LIBS1 := $(LIBS1) -lScript1
LIBS2 := $(LIBS2) -lScript2
endif

BASE_HEADERS = \
	BaseScript.h \
	BaseTrap.h
BASE_OBJS = \
	$(bindir)/MsgHandlerArray.o
BASE_OBJS1 = \
	$(bin1dir)/BaseScript.o \
	$(bin1dir)/BaseTrap.o \
	$(bin1dir)/utils.o
BASE_OBJS2 = \
	$(bin2dir)/BaseScript.o \
	$(bin2dir)/BaseTrap.o \
	$(bin2dir)/utils.o
$(bin1dir)/BaseScript.o: BaseScript.h Script.h ScriptModule.h MsgHandlerArray.h
$(bin2dir)/BaseScript.o: BaseScript.h Script.h ScriptModule.h MsgHandlerArray.h
$(bin1dir)/BaseTrap.o: BaseTrap.h BaseScript.h Script.h
$(bin2dir)/BaseTrap.o: BaseTrap.h BaseScript.h Script.h

KDSCRIPT_HEADERS = \
	CustomHUD.h \
	Rendering.h \
	Text.h \
	Other.h
KDSCRIPT_OBJS1 = \
	$(bin1dir)/CustomHUD.o \
	$(bin1dir)/Rendering.o \
	$(bin1dir)/Text.o \
	$(bin1dir)/Other.o
KDSCRIPT_OBJS2 = \
	$(bin2dir)/CustomHUD.o \
	$(bin2dir)/Rendering.o \
	$(bin2dir)/Text.o \
	$(bin2dir)/Other.o
$(bin1dir)/CustomHUD.o: CustomHUD.h BaseScript.h Script.h scriptvars.h ScriptModule.h utils.h
$(bin2dir)/CustomHUD.o: CustomHUD.h BaseScript.h Script.h scriptvars.h ScriptModule.h utils.h
$(bin1dir)/Rendering.o: Rendering.h BaseScript.h Script.h BaseTrap.h scriptvars.h utils.h
$(bin2dir)/Rendering.o: Rendering.h BaseScript.h Script.h BaseTrap.h scriptvars.h utils.h
$(bin1dir)/Text.o: Text.h BaseScript.h Script.h scriptvars.h CustomHUD.h utils.h
$(bin2dir)/Text.o: Text.h BaseScript.h Script.h scriptvars.h CustomHUD.h utils.h
$(bin1dir)/Other.o: Other.h BaseScript.h Script.h BaseTrap.h utils.h
$(bin2dir)/Other.o: Other.h BaseScript.h Script.h BaseTrap.h utils.h

MODULE_OBJS = \
	$(bindir)/Allocator.o \
	$(bindir)/Script.o \
	$(bindir)/ScriptModule.o \
	$(bindir)/exports.o
MODULE_OBJS1 = \
	$(bin1dir)/ScriptDef.o \
	$(bin1dir)/script_res.o
MODULE_OBJS2 = \
	$(bin2dir)/ScriptDef.o \
	$(bin2dir)/script_res.o
$(bindir)/Allocator.o: Allocator.h
$(bindir)/Script.o: Script.h
$(bindir)/ScriptModule.o: ScriptModule.h Allocator.h
$(bin1dir)/ScriptDef.o: $(BASE_HEADERS) $(KDSCRIPT_HEADERS) ScriptModule.h genscripts.h
$(bin2dir)/ScriptDef.o: $(BASE_HEADERS) $(KDSCRIPT_HEADERS) ScriptModule.h genscripts.h
$(bin1dir)/script_res.o: script.rc version.rc
$(bin2dir)/script_res.o: script.rc version.rc

$(bindir):
	mkdir -p $@

$(bin1dir):
	mkdir -p $@

$(bin2dir):
	mkdir -p $@

$(bindir)/%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(DEFINES) $(CXXDEFINES) $(INCLUDES) -o $@ -c $<

$(bin1dir)/%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(DEFINES) $(DEFINES1) $(CXXDEFINES) $(INCLUDES) -o $@ -c $<

$(bin2dir)/%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(DEFINES) $(DEFINES2) $(CXXDEFINES) $(INCLUDES) -o $@ -c $<

$(bindir)/exports.o: $(bindir)/ScriptModule.o
	$(DLLTOOL) $(DLLFLAGS) --dllname $(MODULE_NAME).osm --output-exp $@ $^

$(bin1dir)/%_res.o: %.rc
	$(RC) $(DEFINES) $(DEFINES1) $(RCDEFINES) -o $@ -i $<

$(bin2dir)/%_res.o: %.rc
	$(RC) $(DEFINES) $(DEFINES2) $(RCDEFINES) -o $@ -i $<

$(MODULE_NAME)-t1.osm: $(BASE_OBJS) $(BASE_OBJS1) $(KDSCRIPT_OBJS1) $(MODULE_OBJS) $(MODULE_OBJS1)
	$(LD) $(LDFLAGS) -Wl,--image-base=0x14700000 $(LIBDIRS) -o $@ script.def $^ $(LIBS1) $(LIBS)

$(MODULE_NAME)-t2.osm: $(BASE_OBJS) $(BASE_OBJS2) $(KDSCRIPT_OBJS2) $(MODULE_OBJS) $(MODULE_OBJS2)
	$(LD) $(LDFLAGS) -Wl,--image-base=0x14700000 $(LIBDIRS) -o $@ script.def $^ $(LIBS2) $(LIBS)

all: $(bindir) $(bin1dir) $(bin2dir) $(MODULE_NAME)-t1.osm $(MODULE_NAME)-t2.osm

clean:
	$(RM) $(MODULE_NAME)-t1.osm $(MODULE_NAME)-t2.osm $(bindir)/*.o $(bin1dir)/*.o $(bin2dir)/*.o

