/******************************************************************************
 *  KDSnuffable.cc
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

#include "KDSnuffable.hh"

KDSnuffable::KDSnuffable (const String& _name, const Object& _host)
	: Script (_name, _host),
	  THIEF_PERSISTENT_FULL (on_mode, AnimLight::Mode::MAXIMUM),
	  THIEF_PERSISTENT_FULL (off_mode, AnimLight::Mode::ZERO),
	  THIEF_PARAMETER (relight_on_frob, false),
	  THIEF_PARAMETER (light_on_schema, Object::NONE),
	  THIEF_PARAMETER (light_off_schema, Object::NONE)
{
	listen_message ("PostSim", &KDSnuffable::on_post_sim);

	listen_message ("TurnOn", &KDSnuffable::turn_on);
	listen_message ("Light", &KDSnuffable::turn_on);
	listen_message ("FireStimStimulus", &KDSnuffable::turn_on);

	listen_message ("TurnOff", &KDSnuffable::turn_off);
	listen_message ("Extinguish", &KDSnuffable::turn_off);
	listen_message ("KOGasStimulus", &KDSnuffable::turn_off);
	listen_message ("WaterStimStimulus", &KDSnuffable::turn_off);

	listen_message ("Toggle", &KDSnuffable::toggle);
	listen_message ("FrobWorldEnd", &KDSnuffable::toggle);
}

void
KDSnuffable::initialize ()
{
	Script::initialize ();
	if (!on_mode.exists () || !off_mode.exists ())
	{
		AnimLight::Mode mode = host_as<AnimLight> ().light_mode;
		switch (mode)
		{
		case AnimLight::Mode::SMOOTH_BRIGHTEN:
		case AnimLight::Mode::SMOOTH_DIM:
			throw std::runtime_error ("This script does not support "
				"the 'smoothly dim' and 'smoothly brighten' "
				"animated light modes.");
		case AnimLight::Mode::MINIMUM:
		case AnimLight::Mode::ZERO:
			on_mode = AnimLight::Mode::MAXIMUM;
			off_mode = mode;
			break;
		default:
			on_mode = mode;
			off_mode = AnimLight::Mode::ZERO;
			break;
		}
	}
}

Message::Result
KDSnuffable::on_post_sim (Message&)
{
	if (host_as<AnimLight> ().light_mode == on_mode)
		on_common ();
	else
		off_common ();

	return Message::HALT;
}

Message::Result
KDSnuffable::turn_on (Message&)
{
	auto light = host_as<AnimLight> ();
	if (light.light_mode == on_mode) return Message::HALT;

	log (Log::NORMAL, "Lighting snuffable light.");
	light.light_mode = on_mode;
	on_common ();

	if (light_on_schema->exists ())
		SoundSchema (light_on_schema).play (host ());
	else
		SoundSchema::play_by_tags ("Event Activate",
			SoundSchema::Tagged::ON_OBJECT, host ());

	return Message::HALT;
}

void
KDSnuffable::on_common ()
{
	host_as<Rendered> ().self_illumination = 1.0f;

	auto sound = host_as<AmbientHacked> ();
	if (sound.is_ambient_hacked ())
		sound.active = true;

	auto particles = host_as<ParticleGroup> ();
	if (particles.is_particle_group ())
		particles.active = true;

	auto models = host_as<ModelsTweq> ();
	if (models.has_models_tweq () && !models.use_model_5)
	{
		host_as<Rendered> ().model.remove ();
		models.active = true;
	}

	GenericMessage ("TurnOn").broadcast (host (), "ControlDevice");
	GenericMessage ("TurnOn").broadcast (host (), "~ParticleAttachement");

	if (!relight_on_frob)
		host ().remove_metaprop (Object ("FrobInert"));
}

Message::Result
KDSnuffable::turn_off (Message&)
{
	auto light = host_as<AnimLight> ();
	if (light.light_mode == off_mode) return Message::HALT;

	log (Log::NORMAL, "Extinguishing snuffable light.");
	light.light_mode = off_mode;
	off_common ();

	if (light_off_schema->exists ())
		SoundSchema (light_off_schema).play (host ());
	else
		SoundSchema::play_by_tags ("Event Deactivate",
			SoundSchema::Tagged::ON_OBJECT, host ());

	return Message::HALT;
}

void
KDSnuffable::off_common ()
{
	host_as<Rendered> ().self_illumination = 0.0f;

	auto sound = host_as<AmbientHacked> ();
	if (sound.is_ambient_hacked ())
		sound.active = false;

	auto particles = host_as<ParticleGroup> ();
	if (particles.is_particle_group ())
		particles.active = false;

	auto models = host_as<ModelsTweq> ();
	if (models.has_models_tweq () && !models.use_model_5)
	{
		host_as<Rendered> ().model = models.model [5];
		models.active = false;
	}

	GenericMessage ("TurnOff").broadcast (host (), "ControlDevice");
	GenericMessage ("TurnOff").broadcast (host (), "~ParticleAttachement");

	if (!relight_on_frob)
		host ().add_metaprop (Object ("FrobInert"));
}

Message::Result
KDSnuffable::toggle (Message& message)
{
	if (host_as<AnimLight> ().light_mode == on_mode)
		return turn_off (message);
	else
		return turn_on (message);
}

