/***************************************************************************
 *  test_powerset.cpp - Test powerset generation
 *
 *  Created:   Wed 19 Jan 14:49:40 CET 2022
 *  Copyright  2022  Till Hofmann <hofmann@kbsg.rwth-aachen.de>
 *  SPDX-License-Identifier: LGPL-3.0-or-later
 ****************************************************************************/


#include "utilities/powerset.h"

#include <catch2/catch_test_macros.hpp>
#include <set>
#include <string>

using tacos::utilities::construct_combinations;
using tacos::utilities::construct_powerset;

TEST_CASE("Power set", "[utilities]")
{
	std::set<std::string> input{"a", "b"};
	CHECK(construct_powerset(input) == std::set<std::set<std::string>>{{}, {"a"}, {"b"}, {"a", "b"}});
	CHECK(construct_powerset(std::set<std::string>{}) == std::set<std::set<std::string>>{{}});
}

TEST_CASE("Combinations of parameters", "[utilities]")
{
	using Params = std::vector<std::vector<std::string>>;
	CHECK(construct_combinations(Params{{"a", "b"}, {"c", "d"}})
	      == Params{{"a", "c"}, {"a", "d"}, {"b", "c"}, {"b", "d"}});
	CHECK(construct_combinations(Params{{"a"}, {"c", "d"}}) == Params{{"a", "c"}, {"a", "d"}});
}
