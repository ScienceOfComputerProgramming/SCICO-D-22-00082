/***************************************************************************
 *  adapter.h - General plant adapter definition required for search
 *
 *  Created:   Tue 19 Oct 17:38:25 CEST 2021
 *  Copyright  2021  Till Hofmann <hofmann@kbsg.rwth-aachen.de>
 *  SPDX-License-Identifier: LGPL-3.0-or-later
 ****************************************************************************/

#pragma once

#include "search/canonical_word.h"

#include <map>

namespace tacos::search {

/** Generic functor to get the next canonical word.
 * This is not actually an implementation, but only defines the interface. Instead, a plant-specific
 * (partial) template specialization needs to be defined that computes the next canonical words.
 */
template <typename Plant,
          typename ActionType,
          typename ConstraintSymbolType,
          bool use_location_constraints = false,
          bool use_set_semantics        = false>
class get_next_canonical_words
{
public:
	get_next_canonical_words(const std::set<ActionType> & = {}, const std::set<ActionType> & = {})
	{
	}
	/** Get all successors for one particular time successor. */
	std::multimap<ActionType, CanonicalABWord<typename Plant::Location, ConstraintSymbolType>>
	operator()(
	  const Plant &,
	  const automata::ata::AlternatingTimedAutomaton<logic::MTLFormula<ConstraintSymbolType>,
	                                                 logic::AtomicProposition<ConstraintSymbolType>>
	    &,
	  const std::pair<typename Plant::Configuration, ATAConfiguration<ConstraintSymbolType>> &,
	  const RegionIndex,
	  const RegionIndex)
	{
		throw std::logic_error("Missing specialization for get_next_canonical_words, did you forget to "
		                       "include the adapter specialization?");
	};
};

} // namespace tacos::search
