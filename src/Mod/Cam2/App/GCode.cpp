/***************************************************************************
 *   Copyright (c) 2012 Luke Parry    (l.parry@warwick.ac.uk)              *
 *                                                                         *
 *   This file is part of the FreeCAD CAx development system.              *
 *                                                                         *
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU Library General Public           *
 *   License as published by the Free Software Foundation; either          *
 *   version 2 of the License, or (at your option) any later version.      *
 *                                                                         *
 *   This library  is distributed in the hope that it will be useful,      *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU Library General Public License for more details.                  *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this library; see the file COPYING.LIB. If not,    *
 *   write to the Free Software Foundation, Inc., 59 Temple Place,         *
 *   Suite 330, Boston, MA  02111-1307, USA                                *
 *                                                                         *
 ***************************************************************************/


#include <PreCompiled.h>
#ifndef _PreComp_
#endif

// #include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>

#include "GCode.h"
using namespace Cam;
GCode::GCode()
{
}

GCode::~GCode()
{
}



void callback(char c, double f)
{
	int i = 3;
}

std::vector<double> doubles;
void add_argument(double const &value)
{
	doubles.push_back(value);
}


namespace qi = boost::spirit::qi;
namespace ascii = boost::spirit::ascii;

typedef std::string::iterator Iterator;
typedef std::pair<char, double> ArgumentData_t;
typedef double Double_t;
Double_t g_double;


template <typename Iterator>
struct point_double_grammar : boost::spirit::qi::grammar<Iterator, double()>
{
    point_double_grammar() : point_double_grammar::base_type(d)
    {
		d = boost::spirit::qi::double_ [ g_double = boost::spirit::qi::as<double>(boost::spirit::qi::_1) ];
    }
    boost::spirit::qi::rule<Iterator, double()> d;
};



namespace qi = boost::spirit::qi;
namespace phx = boost::phoenix;

typedef qi::symbols<char, double> arguments_dictionary;
qi::rule<std::string::const_iterator, qi::space_type> GetRapid()
{
	return(qi::rule<std::string::const_iterator, qi::space_type>(qi::lexeme [+qi::char_("gG") >> *qi::char_("0") >> qi::char_("0")] >> (qi::lexeme [+qi::char_("xXyYzZ")] >> qi::double_)));
}


template <typename Iter, typename Skipper = qi::blank_type> 
	struct rs274 : qi::grammar<Iter, Skipper> 
{

	void Add(std::vector<char> c, double value)
	{
		if (c.size() == 1)
		{
			m_doubles.insert(std::make_pair(c[0], value));
		}
	}

	void Print()
	{
		for (DoubleMap_t::const_iterator itArg = m_doubles.begin(); itArg != m_doubles.end(); itArg++)
		{
			qDebug("%c=%lf\n", itArg->first, itArg->second);
		}
	}

	void ProcessBlock()
	{
		int j=3;
	}

	rs274(arguments_dictionary &dict) : rs274::base_type(Start)
	{
		// Variables declared here are created and destroyed for each rule parsed.
		// Use member variables of the structure for long-lived variables instead.

		// arguments_dictionary arguments;

		line_number = 0;

		// N110 - i.e. use qi::lexeme to avoid qi::space_type skipper which allows the possibility of interpreting 'N 110'.  We don't want spaces here.
		LineNumberRule = (qi::lexeme [qi::char_("nN") ] >> qi::int_ )
			[ phx::ref(line_number) = qi::_2 ];

		// X 1.1 etc.
		MotionArgument = (qi::lexeme [+qi::char_("xXyYzZ")] >> qi::double_)
			[ phx::bind(&rs274<Iter, Skipper>::Add, phx::ref(*this), qi::_1, qi::_2) ] // call this->Add(_1, _2);
			;

		// g00 X <float> Y <float> etc.
		Rapid = (qi::lexeme [+qi::char_("gG") >> *qi::char_("0") >> qi::char_("0")] >> +(MotionArgument) >> EndOfBlock)
			[ phx::bind(&rs274<Iter, Skipper>::Print, phx::ref(*this) ) ]	// call this->Print()
			;

		// g01 X <float> Y <float> etc.
		Feed = (qi::lexeme [+qi::char_("gG") >> *qi::char_("0") >> qi::char_("1")] >> +(MotionArgument))
			[ phx::bind(&rs274<Iter, Skipper>::Print, phx::ref(*this) ) ]	// call this->Print()
			;

			EndOfBlock = (qi::no_skip[*qi::space >> qi::eol])
			[ phx::bind(&rs274<Iter, Skipper>::ProcessBlock, phx::ref(*this) ) ]	// call this->EndOfBlock()
			;

		MotionCommand =	
					(LineNumberRule >> EndOfBlock)
				|	(LineNumberRule >> Feed >> EndOfBlock)
				|	(LineNumberRule >> Rapid)
					;

		Start = +(MotionCommand)	// [ phx::bind(&Arguments_t::Print, phx::ref(arguments) ) ]
					;

		BOOST_SPIRIT_DEBUG_NODE(Start);
		BOOST_SPIRIT_DEBUG_NODE(MotionArgument);
		BOOST_SPIRIT_DEBUG_NODE(Feed);
		BOOST_SPIRIT_DEBUG_NODE(Rapid);
		BOOST_SPIRIT_DEBUG_NODE(MotionCommand);
		BOOST_SPIRIT_DEBUG_NODE(LineNumberRule);
		BOOST_SPIRIT_DEBUG_NODE(EndOfBlock);
	}

	public:
		qi::rule<Iter, Skipper> Start;
		qi::rule<Iter, Skipper> MotionArgument;
		qi::rule<Iter, Skipper> Feed;
		qi::rule<Iter, Skipper> Rapid;		
		qi::rule<Iter, Skipper> MotionCommand;
		qi::rule<Iter, Skipper> LineNumberRule;
		qi::rule<Iter, Skipper> EndOfBlock;

		int			line_number;

		typedef std::map<char, double>	DoubleMap_t;
		DoubleMap_t	m_doubles;

		typedef std::map<char, int>	IntegerMap_t;
		IntegerMap_t	m_integers;
};

int CamExport wilma()
{

	// from http://stackoverflow.com/questions/12208705/add-to-a-spirit-qi-symbol-table-in-a-semantic-action
	// and http://stackoverflow.com/questions/9139015/parsing-mixed-values-and-key-value-pairs-with-boost-spirit



	arguments_dictionary arguments;
	rs274<std::string::const_iterator> linuxcnc(arguments);

	const std::string gcode = "N220 g0 X 1.1 Y 2.2 Z3.3   \n";

	std::string::const_iterator begin = gcode.begin();
	// if (qi::phrase_parse(begin, gcode.end(), linuxcnc, qi::space - qi::eol))
	if (qi::phrase_parse(begin, gcode.end(), linuxcnc, qi::blank))
	{
		if (arguments.find("X")) qDebug("%lf\n", arguments.at("X"));
		if (arguments.find("Y")) qDebug("%lf\n", arguments.at("Y"));
		if (arguments.find("Z")) qDebug("%lf\n", arguments.at("Z"));
	}
	else
	{
		qDebug("Parsing failed at N%d %d, %s\n", linuxcnc.line_number, std::distance(gcode.begin(), begin), begin);
	}

	return(0);
}