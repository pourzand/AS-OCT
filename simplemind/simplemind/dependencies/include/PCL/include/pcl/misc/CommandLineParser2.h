#ifndef PCL_COMMAND_LINE_PARSER
#define PCL_COMMAND_LINE_PARSER

#include <pcl/misc/StringTokenizer.h>
#include <pcl/exception.h>
#include <pcl/macro.h>
#include <boost/lexical_cast.hpp>

#pragma warning ( push )
#pragma warning ( disable : 4101 )

namespace pcl
{
	namespace command_line_tester
	{
		class LexicalCastTester
	}
	
	namespace command_line_parser_details
	{
		class ArgumentTesterBase
		{
			virtual bool operator()(std::string)
		};

		class Parameter
		{
		};
	}

	class CommandLineParser
	{
	};
}

#endif