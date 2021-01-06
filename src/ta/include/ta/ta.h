/***************************************************************************
 *  ta.h - Core functionality for timed automata
 *
 *  Created: Tue 26 May 2020 13:44:41 CEST 13:44
 *  Copyright  2020  Till Hofmann <hofmann@kbsg.rwth-aachen.de>
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

#pragma once

#include "automata.h"

#include <algorithm>
#include <map>
#include <set>
#include <string>
#include <tuple>
#include <vector>

namespace automata {

namespace ta {

using ClockSetValuation = std::map<std::string, Time>;
template <typename LocationT>
using Configuration = std::pair<LocationT, ClockSetValuation>;

template <typename LocationT>
class TimedAutomaton;

/// A transition in a timed automaton.
/** @see TimedAutomaton
 */
template <typename LocationT>
class Transition
{
public:
	friend class TimedAutomaton<LocationT>;
	/** Constructor.
	 * @param source the source location
	 * @param symbol the symbol to read with this transition
	 * @param target the target location
	 * @param clock_constraints A map defining the constraints of the clock,
	 *        where the key specifies the name of the clock and the value is a
	 *        constraint on that clock
	 * @param clock_resets the set of clocks to reset on this transition
	 */
	Transition(const LocationT &                                  source,
	           const Symbol &                                     symbol,
	           const LocationT &                                  target,
	           const std::multimap<std::string, ClockConstraint> &clock_constraints = {},
	           const std::set<std::string> &                      clock_resets      = {})
	: source_(source),
	  target_(target),
	  symbol_(symbol),
	  clock_constraints_(clock_constraints),
	  clock_resets_(clock_resets)
	{
	}
	/** Check whether the transition is enabled on the given symbol and clock valuations.
	 * More specifically, check if the given symbol matches this transition's symbol, and that the
	 * clock valuations satisfy all clock constraints.
	 * @param symbol The symbol to check
	 * @param clock_vals The clock valuations, given as map with the clock names as keys and the
	 * clocks as value
	 * @return true if the transition can be taken.
	 */
	bool
	is_enabled(const Symbol &symbol, const std::map<std::string, Clock> &clock_vals) const
	{
		if (symbol != symbol_) {
			return false;
		}
		return std::all_of(std::begin(clock_constraints_),
		                   std::end(clock_constraints_),
		                   [&clock_vals](const auto &constraint) {
			                   return is_satisfied(constraint.second,
			                                       clock_vals.at(constraint.first).get_valuation());
		                   });
	}

private:
	const LocationT                                   source_;
	const LocationT                                   target_;
	const Symbol                                      symbol_;
	const std::multimap<std::string, ClockConstraint> clock_constraints_;
	const std::set<std::string>                       clock_resets_;
};

/// One specific (finite) path in the timed automaton.
template <typename LocationT>
class Path
{
public:
	friend class TimedAutomaton<LocationT>;
	/// Compare two paths of a TA
	friend bool
	operator<(const Path<LocationT> &p1, const Path<LocationT> &p2)
	{
		return p1.sequence_ < p2.sequence_;
	}
	/// Constructor
	/** Start a new path in the given initial location with the given clocks.
	 * @param initial_location the initial location of the path, should be the same as the TA's
	 * initial location
	 * @param clocks a set of clock names, should be names of the TA's clocks
	 */
	Path(LocationT initial_location, std::set<std::string> clocks)
	: current_location_(initial_location), tick_(0)
	{
		for (const auto &clock : clocks) {
			clock_valuations_.emplace(std::make_pair(clock, Clock()));
		}
	}

private:
	std::vector<std::tuple<Symbol, Time, LocationT>> sequence_;
	std::map<std::string, Clock>                     clock_valuations_;
	LocationT                                        current_location_;
	Time                                             tick_;
};

/// A timed automaton.
/** A TimedAutomaton consists of a set of locations, an initial location, a final location, a set of
 * clocks, and a set of transitions. A simple timed automaton with two locations and a single
 * transition without constraints can be constructed with
 * @code
 * TimedAutomaton ta{"s0", {"s1"}};
 * ta.add_transition(Transition("s0", "a", "s1"));
 * @endcode
 * To construct a timed automaton with a clock constraint <tt>x < 1</tt>, use
 * @code
 * TimedAutomaton ta{"s0", {"s1"}};
 * ClockConstraint c = AtomicClockConstraintT<std::less<Time>>(1);
 * ta.add_transition(Transition("s0", "a", "s1", {{"x", c}}));
 * @endcode
 */
template <typename LocationT>
class TimedAutomaton
{
public:
	TimedAutomaton() = delete;
	/** Constructor.
	 * @param initial_location the initial location
	 * @param final_locations a set of final locations
	 */
	TimedAutomaton(const LocationT &initial_location, const std::set<LocationT> &final_locations)
	: locations_{initial_location},
	  initial_location_(initial_location),
	  final_locations_(final_locations)
	{
		add_locations(final_locations_);
	}
	/** Add a location to the TA.
	 * @param location the location to add
	 */
	void
	add_location(const LocationT &location)
	{
		locations_.insert(location);
	}
	/** Add a clock to the TA.
	 * @param name the name of the clock
	 */
	void
	add_clock(const std::string &name)
	{
		clocks_.insert(name);
	}
	/** Add a set of locations to the TA
	 * @param locations the locations to add
	 */
	void
	add_locations(const std::set<LocationT> &locations)
	{
		for (const auto &location : locations) {
			add_location(location);
		}
	}
	/** Add a transition to the TA.
	 * @param transition The transition to add, must only mention clocks and locations that are
	 * already part of the TA.
	 */
	void
	add_transition(const Transition<LocationT> &transition)
	{
		if (!locations_.count(transition.source_)) {
			throw InvalidLocationException(transition.source_);
		}
		if (!locations_.count(transition.target_)) {
			throw InvalidLocationException(transition.target_);
		}
		for (const auto &[clock_name, constraint] : transition.clock_constraints_) {
			if (!clocks_.count(clock_name)) {
				throw InvalidClockException(clock_name);
			};
		}
		for (const auto &clock_name : transition.clock_resets_) {
			if (!clocks_.count(clock_name)) {
				throw InvalidClockException(clock_name);
			};
		}
		transitions_.insert({transition.source_, transition});
	}
	/// Let the TA make a transition on the given symbol at the given time.
	/** Check if there is a transition that can be enabled on the given symbol at the given time,
	 * starting with the given path. If so, modify the given path, i.e., apply the transition by
	 * switching to the new location, increasing all clocks by the time difference, and resetting all
	 * clocks specified in the transition. This always uses the first transition that is enabled,
	 * i.e., it does not work properly on non-deterministic TAs.
	 * @param path The path prefix to start at
	 * @param symbol The symbol to read
	 * @param time The (absolute) time associated with the symbol
	 * @return a (possibly empty) set of valid paths after applying the transition
	 */
	std::set<Path<LocationT>>
	make_transition(Path<LocationT> path, const Symbol &symbol, const Time &time) const
	{
		if (path.tick_ > time) {
			return {};
		}
		for (auto &[name, clock] : path.clock_valuations_) {
			clock.tick(time - path.tick_);
		}
		path.tick_         = time;
		auto [first, last] = transitions_.equal_range(path.current_location_);
		std::set<Path<LocationT>> paths;
		while (first != last) {
			auto trans = std::find_if(first, last, [&](const auto &trans) {
				return trans.second.is_enabled(symbol, path.clock_valuations_);
			});
			if (trans == last) {
				break;
			}
			first                  = std::next(trans);
			path.current_location_ = trans->second.target_;
			path.sequence_.push_back(std::make_tuple(symbol, time, path.current_location_));
			for (const auto &name : trans->second.clock_resets_) {
				path.clock_valuations_[name].reset();
			}
			paths.insert(path);
		}
		return paths;
	}
	/// Check if the TA accepts the given timed word.
	/** Iteratively apply transitions for each (symbol,time) pair in the given timed word.
	 * @param word the word to read
	 * @return true if the word was accepted
	 */
	bool
	accepts_word(const TimedWord &word) const
	{
		std::set<Path<LocationT>> paths{Path{initial_location_, clocks_}};
		for (auto &[symbol, time] : word) {
			std::set<Path<LocationT>> res_paths;
			for (auto &path : paths) {
				auto new_paths = make_transition(path, symbol, time);
				res_paths.insert(std::begin(new_paths), std::end(new_paths));
			}
			paths = res_paths;
			if (paths.empty()) {
				return false;
			}
		}
		for (auto &path : paths) {
			if (final_locations_.find(path.current_location_) != final_locations_.end()) {
				return true;
			};
		}
		return false;
	}

private:
	std::set<LocationT>                             locations_;
	const LocationT                                 initial_location_;
	const std::set<LocationT>                       final_locations_;
	std::set<std::string>                           clocks_;
	std::multimap<LocationT, Transition<LocationT>> transitions_;
};

///// Compare two paths of a TA
// template <typename LocationT>
// bool
// operator<(const Path<LocationT> &p1, const Path<LocationT> &p2)
//{
//	return p1.sequence_ < p2.sequence_;
//}

} // namespace ta
} // namespace automata
