/******************************************************************************
 *  KDOptionalReverse.cc
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

#include "KDOptionalReverse.hh"

KDOptionalReverse::KDOptionalReverse (const String& _name, const Object& _host)
	: Script (_name, _host)
{
	listen_message ("PostSim", &KDOptionalReverse::on_post_sim);
#ifdef IS_THIEF2
	listen_message ("QuestChange", &KDOptionalReverse::on_quest_change);
	listen_message ("EndScript", &KDOptionalReverse::on_end_script);
#endif // IS_THIEF2
}

#ifdef IS_THIEF2

Message::Result
KDOptionalReverse::on_post_sim (Message&)
{
	// Subscribe to objectives with negations.
	for (Objective objective = 0; objective.exists (); ++objective.number)
		if (get_negation (objective).number != Objective::NONE)
			objective.subscribe (host (), Objective::Field::STATE);

	return Message::HALT;
}

Message::Result
KDOptionalReverse::on_quest_change (QuestChangeMessage& message)
{
	if (message.get_old_value () == message.get_new_value ())
		return Message::HALT;

	// Translate from the objective to its negation.
	auto objective =
		Objective::parse_quest_var (message.get_quest_var ());
	if (objective.field == Objective::Field::STATE)
		update_negation (objective.number, false);

	return Message::HALT;
}

Message::Result
KDOptionalReverse::on_end_script (Message&)
{
	// Fix anything that VictoryCheck did incorrectly.
	for (Objective objective = 0; objective.exists (); ++objective.number)
		update_negation (objective, true);

	return Message::HALT;
}

Objective
KDOptionalReverse::get_negation (Objective objective)
{
	std::ostringstream _negation;
	_negation << "goal_negation_" << objective.number;
	QuestVar negation (_negation.str ());
	return negation.get (Objective::NONE);
}

void
KDOptionalReverse::update_negation (Objective objective, bool final)
{
	Objective negation = get_negation (objective);
	if (negation.number == Objective::NONE) return;

	Objective::State obj_state = objective.get_state (), neg_state;
	switch (obj_state)
	{
	case Objective::State::INCOMPLETE:
		neg_state = final
			? Objective::State::COMPLETE // conditions never met
			: Objective::State::INCOMPLETE; // still awaiting result
		break;
	case Objective::State::COMPLETE: // conditions were met, so breached
	case Objective::State::CANCELLED: // cancelled but not by VictoryCheck
		neg_state = Objective::State::CANCELLED;
		break;
	case Objective::State::FAILED: // this shouldn't happen, but hey
	default: // neither should this, of course
		neg_state = Objective::State::FAILED;
		break;
	}

	negation.set_state (neg_state);
}

#else // !IS_THIEF2

Message::Result
KDOptionalReverse::on_post_sim (Message&)
{
	mono () << "Error: This script is not available for this game." << std::endl;
	return Message::ERROR;
}

#endif // IS_THIEF2

