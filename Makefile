#******************************************************************************
#   Makefile
#
#   Copyright (C) 2012-2013 Kevin Daughtridge <kevin@kdau.com>
#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 3 of the License, or
#   (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with this program. If not, see <http://www.gnu.org/licenses/>.
#
#******************************************************************************

MODULE_NAME = KDScript
HOST = x86_64-linux-gnu
TARGET = i686-w64-mingw32
THIEFLIBDIR = ThiefLib
default: thief1 thief2

SCRIPT_HEADERS = \
	KDCarried.hh \
	KDCarrier.hh \
	KDGetInfo.hh \
	KDHUDElement.hh \
	KDJunkTool.hh \
	KDOptionalReverse.hh \
	KDQuestArrow.hh \
	KDRenewable.hh \
	KDShortText.hh \
	KDStatMeter.hh \
	KDSubtitled.hh \
	KDSyncGlobalFog.hh \
	KDToolSight.hh \
	KDTransitionTrap.hh \
	KDTrapEnvMap.hh \
	KDTrapFog.hh \
	KDTrapNextMission.hh \
	KDTrapWeather.hh

include $(THIEFLIBDIR)/module.mk

$(bindir1)/KDQuestArrow.o: KDHUDElement.hh
$(bindir2)/KDQuestArrow.o: KDHUDElement.hh
$(bindir1)/KDStatMeter.o: KDHUDElement.hh
$(bindir2)/KDStatMeter.o: KDHUDElement.hh
$(bindir1)/KDToolSight.o: KDHUDElement.hh
$(bindir2)/KDToolSight.o: KDHUDElement.hh

$(bindir1)/KDSyncGlobalFog.o: KDTransitionTrap.hh
$(bindir2)/KDSyncGlobalFog.o: KDTransitionTrap.hh
$(bindir1)/KDTrapFog.o: KDTransitionTrap.hh
$(bindir2)/KDTrapFog.o: KDTransitionTrap.hh
$(bindir1)/KDTrapWeather.o: KDTransitionTrap.hh
$(bindir2)/KDTrapWeather.o: KDTransitionTrap.hh

