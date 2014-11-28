/******************************************************************************
 *  module.cc
 *
 *  Copyright (C) 2012-2014 Kevin Daughtridge <kevin@kdau.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 *****************************************************************************/

#include <Thief/Thief.hh>
#include "version.rc"

#include "KDCarried.hh"
#include "KDCarrier.hh"
#include "KDGetInfo.hh"
#include "KDHUDElement.hh"
#include "KDJunkTool.hh"
#include "KDOptionalReverse.hh"
#include "KDQuestArrow.hh"
#include "KDRenewable.hh"
#include "KDShortText.hh"
#include "KDStatMeter.hh"
#include "KDSubtitled.hh"
#include "KDSyncGlobalFog.hh"
#include "KDToolSight.hh"
#include "KDTrapEnvMap.hh"
#include "KDTrapFog.hh"
#include "KDTrapNextMission.hh"
#include "KDTrapWeather.hh"

class KDScriptDemo : public Script
{
public:
	KDScriptDemo (const String& name, const Object& host);

private:
	Message::Result on_sim (SimMessage&);
};

THIEF_MODULE (MODULE_NAME,
	THIEF_SCRIPT ("KDCarried", "Script", KDCarried),
	THIEF_SCRIPT ("KDCarrier", "Script", KDCarrier),
	THIEF_SCRIPT ("KDGetInfo", "Script", KDGetInfo),
	THIEF_SCRIPT ("KDJunkTool", "Script", KDJunkTool),
	THIEF_SCRIPT ("KDOptionalReverse", "Script", KDOptionalReverse),
	THIEF_SCRIPT ("KDQuestArrow", "KDHUDElement", KDQuestArrow),
	THIEF_SCRIPT ("KDRenewable", "Script", KDRenewable),
	THIEF_SCRIPT ("KDShortText", "Script", KDShortText),
	THIEF_SCRIPT ("KDStatMeter", "KDHUDElement", KDStatMeter),
	THIEF_SCRIPT ("KDSubtitledAI", "KDSubtitled", KDSubtitledAI), // deprecated
	THIEF_SCRIPT ("KDSubtitledVO", "KDSubtitled", KDSubtitledVO), // deprecated
	THIEF_SCRIPT ("KDSyncGlobalFog", "Script", KDSyncGlobalFog),
	THIEF_SCRIPT ("KDToolSight", "KDHUDElement", KDToolSight),
	THIEF_SCRIPT ("KDTrapEnvMap", "TrapTrigger", KDTrapEnvMap),
	THIEF_SCRIPT ("KDTrapFog", "TrapTrigger", KDTrapFog),
	THIEF_SCRIPT ("KDTrapNextMission", "TrapTrigger", KDTrapNextMission),
	THIEF_SCRIPT ("KDTrapWeather", "TrapTrigger", KDTrapWeather),
	THIEF_SCRIPT ("KDScriptDemo", "Script", KDScriptDemo),
)



// KDScriptDemo

KDScriptDemo::KDScriptDemo (const String& _name, const Object& _host)
	: Script (_name, _host)
{
	listen_message ("Sim", &KDScriptDemo::on_sim);
}

Message::Result
KDScriptDemo::on_sim (SimMessage& message)
{
	if (message.event == SimMessage::START)
	{
		for (auto& enable : ScriptParamsLink::get_all_by_data
				(host (), "Enable"))
			enable.get_dest ().remove_metaprop (Object ("FrobInert"));
		host ().destroy ();
	}
	return Message::HALT;
}
