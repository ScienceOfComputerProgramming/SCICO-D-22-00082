/***************************************************************************
 *  benchmark_converyor_belt - case study with a simple conveyor belt model
 *
 *  Created: Mon 26 Jul 2021 20:06:42 CEST 13:00
 *  Copyright  2021  Stefan Schupp <stefan.schupp@tuwien.ac.at>
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

#include "automata/automata.h"
#include "automata/ta.h"
#include "automata/ta_product.h"
#include "automata/ta_regions.h"
#include "mtl/MTLFormula.h"
#include "mtl_ata_translation/translator.h"
#include "railroad.h"
#include "search/canonical_word.h"
#include "search/create_controller.h"
#include "search/heuristics.h"
#include "search/search.h"
#include "search/search_tree.h"
#include "search/synchronous_product.h"
#ifdef VISUALIZATION
#	include "visualization/ta_to_graphviz.h"
#	include "visualization/tree_to_graphviz.h"
#endif

#include <fmt/format.h>
#include <fmt/ostream.h>
#include <spdlog/common.h>
#include <spdlog/spdlog.h>

#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#undef TRUE

namespace {

using Location   = automata::ta::Location<std::string>;
using TA         = automata::ta::TimedAutomaton<std::string, std::string>;
using Transition = automata::ta::Transition<std::string, std::string>;
using automata::Time;
using F  = logic::MTLFormula<std::string>;
using AP = logic::AtomicProposition<std::string>;
using search::NodeLabel;
using TreeSearch = search::TreeSearch<std::string, std::string>;

std::unique_ptr<search::Heuristic<long, search::SearchTreeNode<std::string, std::string>>>
generate_heuristic(long                  weight_canonical_words     = 0,
                   long                  weight_environment_actions = 0,
                   std::set<std::string> environment_actions        = {},
                   long                  weight_time_heuristic      = 1)
{
	using H = search::Heuristic<long, search::SearchTreeNode<std::string, std::string>>;
	std::vector<std::pair<long, std::unique_ptr<H>>> heuristics;
	heuristics.emplace_back(
	  weight_canonical_words,
	  std::make_unique<
	    search::NumCanonicalWordsHeuristic<long,
	                                       search::SearchTreeNode<std::string, std::string>>>());
	heuristics.emplace_back(
	  weight_environment_actions,
	  std::make_unique<
	    search::PreferEnvironmentActionHeuristic<long,
	                                             search::SearchTreeNode<std::string, std::string>,
	                                             std::string>>(environment_actions));
	heuristics.emplace_back(
	  weight_time_heuristic,
	  std::make_unique<
	    search::TimeHeuristic<long, search::SearchTreeNode<std::string, std::string>>>());
	return std::make_unique<
	  search::CompositeHeuristic<long, search::SearchTreeNode<std::string, std::string>>>(
	  std::move(heuristics));
}

TEST_CASE("Conveyor belt", "[benchmark]")
{
	Location l_no{"NO"};
	Location l_st{"ST"};
	Location l_op{"OP"};
	Location l_sp{"SP"};

	std::set<std::string> environment_actions{"release", "resume", "stuck"};
	std::set<std::string> controller_actions{"move", "stop"};
	std::set<std::string> actions;
	std::set_union(std::begin(environment_actions),
	               std::end(environment_actions),
	               std::begin(controller_actions),
	               std::end(controller_actions),
	               std::inserter(actions, std::begin(actions)));

	// the conveyor belt plant
	TA plant{{l_no, l_st, l_op, l_sp},
	         actions,
	         l_no,
	         {l_no},
	         {"stop_timer"},
	         {Transition{l_no, "move", l_no},
	          Transition{l_no, "stuck", l_st},
	          Transition{l_no, "stop", l_sp},
	          Transition{l_st, "release", l_no},
	          Transition{l_st, "release", l_op},
	          Transition{l_op, "stop", l_sp},
	          Transition{l_sp, "resume", l_no}}};

	// the specification
	const auto move_f    = F{AP{"move"}};
	const auto release_f = F{AP{"release"}};
	const auto stuck_f   = F{AP{"stuck"}};
	const auto stop_f    = F{AP{"stop"}};
	const auto resume_f  = F{AP{"resume"}};
	F          spec{move_f.dual_until(!release_f, logic::TimeInterval(0, 2))};

	auto ata = mtl_ata_translation::translate(
	  spec, {AP{"move"}, AP{"release"}, AP{"stuck"}, AP{"stop"}, AP{"resume"}});
	CAPTURE(spec);
	CAPTURE(plant);
	CAPTURE(ata);
	const unsigned int K = std::max(plant.get_largest_constant(), spec.get_largest_constant());
	TreeSearch         search{
    &plant, &ata, controller_actions, environment_actions, K, true, true, generate_heuristic()};
	search.build_tree(false);
	CHECK(search.get_root()->label == NodeLabel::TOP);
#ifdef HAVE_VISUALIZATION
	visualization::search_tree_to_graphviz(*search.get_root(), true)
	  .render_to_file(fmt::format("railroad{}.svg", num_crossings));
	visualization::ta_to_graphviz(controller_synthesis::create_controller(
	                                search.get_root(), controller_actions, environment_actions, 2),
	                              false)
	  .render_to_file(fmt::format("railroad{}_controller.pdf", num_crossings));
#endif
}

} // namespace