/***************************************************************************
 *  test_xml_writer.cpp - Test xml output
 *
 *  Created:   Thu 22 Jul 12:01:49 CEST 2021
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
 *  Read the full text in the LICENSE.md file.
 */

#include "io/XmlWriter.h"

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

namespace {

using Location = automata::ta::Location<std::string>;

using Catch::Matchers::Contains;

TEST_CASE("Write constraint to xml", "[io]")
{
	tinyxml2::XMLDocument doc;
	auto *                root_element = doc.NewElement("root");
	doc.InsertEndChild(root_element);
	automata::ClockConstraint guard_less =
	  automata::AtomicClockConstraintT<std::less<automata::Time>>(3);

	io::add_to_uppaal_xml(std::make_pair("x", guard_less), doc, root_element);

	tinyxml2::XMLPrinter prnt{};
	doc.SaveFile("test.xml");
	doc.Print(&prnt);
	std::string res = std::string(prnt.CStr());
	CHECK(res.find("<root>") != std::string::npos);
	CHECK(res.find("</root>") != std::string::npos);
	CHECK(res.find("<label kind=\"guard\">") != std::string::npos);
	CHECK(res.find("</label>") != std::string::npos);
	CHECK(res.find("x &lt; 3") != std::string::npos);
}

TEST_CASE("Write transition to xml", "[io]")
{
	tinyxml2::XMLDocument doc;
	auto *                root_element = doc.NewElement("root");
	doc.InsertEndChild(root_element);
	automata::ta::Transition<std::string, std::string> transition{"l0", "a", "l1", {}, {}};

	io::add_to_uppaal_xml(transition, doc, root_element);

	tinyxml2::XMLPrinter prnt{};
	doc.SaveFile("test.xml");
	doc.Print(&prnt);
	std::string res = std::string(prnt.CStr());
	CHECK(res.find("<root>") != std::string::npos);
	CHECK(res.find("</root>") != std::string::npos);
	CHECK(res.find("<label kind=\"guard\">") != std::string::npos);
	CHECK(res.find("</label>") != std::string::npos);
	CHECK(res.find("x &lt; 3") != std::string::npos);
}

} // namespace
