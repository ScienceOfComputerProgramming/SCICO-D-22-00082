/***************************************************************************
 *  fischer.cpp - Utility functions for fischer test scenario
 *
 *  Created: Wed 21 Apr 2021 15:07:42 CEST 14:00
 *  Copyright  2021  Stefan Schupp <stefan.schupp@cs.rwth-aachen.de>
 ****************************************************************************/

/*  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  Read the full text in the LICENSE.GPL file in the doc directory.
 */

#include "fischer.h"

#include "automata/automata.h"
#include "automata/ta.h"
#include "automata/ta_product.h"

using automata::Time;
using Location   = automata::ta::Location<std::string>;
using TA         = automata::ta::TimedAutomaton<std::string, std::string>;
using Transition = automata::ta::Transition<std::string, std::string>;
using automata::AtomicClockConstraintT;

std::tuple<automata::ta::TimedAutomaton<std::vector<std::string>, std::string>,
           std::set<std::string>,
           std::set<std::string>>
create_fischer_instance(std::size_t count, Time delay_self_assign, Time delay_enter_critical)
{
	std::vector<TA>         automata;
	std::set<std::string>   controller_actions;
	std::set<std::string>   environment_actions;
	std::set<std::string>   critical_section_actions;
	std::set<Location>      critical_section_locations;
	std::vector<Transition> critical_section_transitions;
	std::vector<Transition> critical_section_interprocess_transitions;
	// critical section setup
	Location free{"FREE"};
	critical_section_locations.insert(free);
	for (std::size_t i = 1; i <= count; ++i) {
		const std::string clock     = "c_" + std::to_string(i);
		const std::string try_enter = "try_enter_" + std::to_string(i);
		const std::string retry     = "retry_" + std::to_string(i);
		const std::string enter     = "enter_" + std::to_string(i);
		const std::string set_var   = "set_var_" + std::to_string(i);
		const std::string zero_var  = "zero_var_" + std::to_string(i);
		environment_actions.insert({try_enter, set_var, zero_var});
		controller_actions.insert({retry, enter});
		// environment_actions.insert({});
		automata.push_back(
		  TA{{Location{"IDLE"}, Location{"REQUEST"}, Location{"CRITICAL"}, Location{"WAIT"}},
		     {try_enter, retry, enter, set_var, zero_var},
		     Location{"IDLE"},
		     {Location{"IDLE"}},
		     {clock},
		     {Transition(Location{"IDLE"}, try_enter, Location{"REQUEST"}, {}, {clock}),
		      Transition(Location{"REQUEST"},
		                 set_var,
		                 Location{"WAIT"},
		                 {{clock, AtomicClockConstraintT<std::less<Time>>(delay_self_assign)}},
		                 {clock}),
		      // Transition(Location{"WAIT"}, retry, Location{"IDLE"}),
		      Transition(Location{"WAIT"},
		                 enter,
		                 Location{"CRITICAL"},
		                 {{clock, AtomicClockConstraintT<std::greater<Time>>(delay_enter_critical)}},
		                 {}),
		      Transition(Location("CRITICAL"), zero_var, Location("IDLE"))}});
		/*
		const std::string i_s = std::to_string(i);
		const Location    in{"IN_" + i_s};
		critical_section_locations.insert(in);
		critical_section_actions.insert({get_near, enter, leave});
		critical_section_transitions.push_back(Transition{free, set_var, in});
		critical_section_transitions.push_back(Transition{free, try_enter, free});
		critical_section_transitions.push_back(Transition{free, retry, free});
		critical_section_transitions.push_back(Transition{in, set_var, in});
		critical_section_transitions.push_back(Transition{in, enter, in});
		critical_section_transitions.push_back(Transition{in, zero_var, free});
		for (std::size_t j = 1; j <= count; ++j) {
		  if (j != i) {
		    critical_section_transitions.push_back(Transition{in, "retry_" + std::to_string(j), in});
		  }
		  critical_section_transitions.push_back(
		    Transition{in, "set_var_" + std::to_string(j), Location("IN_" + std::to_string(j))});
		}
		*/
	}
	// automata.push_back(
	//  TA{critical_section_locations, {}, free, {}, {}, critical_section_transitions});
	return std::make_tuple(automata::ta::get_product(automata),
	                       controller_actions,
	                       environment_actions);
}
