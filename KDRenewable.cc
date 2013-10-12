/******************************************************************************
 *  KDRenewable.cc
 *
 *  Copyright (C) 2013 Kevin Daughtridge <kevin@kdau.com>
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

#include "KDRenewable.hh"

KDRenewable::KDRenewable (const String& _name, const Object& _host)
	: Script (_name, _host),
	  THIEF_PARAMETER_FULL (frequency, "renewable_frequency", 180000ul),
	  THIEF_PARAMETER_FULL (threshold, "renewable_threshold", 0),
	  THIEF_PARAMETER_FULL (physical, "renewable_physical", false)
{
	listen_message ("PostSim", &KDRenewable::on_post_sim);
	listen_timer ("Renew", &KDRenewable::on_renew);
}

Message::Result
KDRenewable::on_post_sim (Message&)
{
	// This non-standard use of the Script->Timing property is kept for
	// backwards compatibility with miss16.osm's RenewableResource.
	Time delay = frequency;
	if (host ().script_timing.exists () && !frequency.exists ())
		delay = 1000ul * Time (host ().script_timing);

	if (delay != 0ul)
	{
		TimerMessage ("Renew").send (host (), host ());
		start_timer ("Renew", delay, true);
	}

	return Message::HALT;
}

Message::Result
KDRenewable::on_renew (TimerMessage&)
{
	Player player;

	// Check for a previously created instance.
	Link previous = Link::get_one ("Owns", host ());
	if (previous != Link::NONE)
	{
		if (player.has_touched (previous.get_dest ()))
			previous.destroy ();
		else
			return Message::HALT;
	}

	// Identify the resource archetype and stack count threshold.
	Object archetype;
	size_t my_threshold = 0u;
	for (auto& script_param : ScriptParamsLink::get_all (host ()))
	{
		if (CIString ("Renewable") == script_param.data)
		{
			my_threshold = threshold;
			archetype = script_param.get_dest ();
			break;
		}

		// For backwards compatibility, look in the link data.
		try
		{
			my_threshold = std::stoul (script_param.data);
			archetype = script_param.get_dest ();
			break;
		}
		catch (...) {}
	}
	if (archetype == Object::NONE)
		return Message::HALT;

	// Transmogrify the archetype for the inventory check.
	Object inv_type = archetype;
	auto transmute = Link::get_all ("Transmute", archetype, Object::ANY,
		Link::Inheritance::SOURCE);
	if (!transmute.empty ())
		inv_type = transmute.front ().get_dest ();
	else if (archetype == Object ("EarthCrystal"))
		inv_type = Object ("EarthArrow");
	else if (archetype == Object ("WaterCrystal"))
		inv_type = Object ("water");
	else if (archetype == Object ("FireCrystal"))
		inv_type = Object ("firearr");
	else if (archetype == Object ("AirCrystal"))
		inv_type = Object ("GasArrow");

	// Check the inventory count.
	size_t inv_count = 0;
	for (auto& content : player.get_inventory ())
		if (content.object.inherits_from (inv_type))
			inv_count += Combinable (content.object).stack_count;
	if (inv_count >= my_threshold)
		return Message::HALT;

	// Create new instance.
	Physical instance = Object::start_create (archetype);
	instance.set_position (Vector (), Vector (), host ());
	Link::create ("Owns", host (), instance);

	// Remove the instance's physics, if required.
	if (!physical)
		instance.remove_physics ();

	instance.finish_create ();
	log (Log::NORMAL, "Created new renewable instance %||.", instance);
	return Message::HALT;
}

