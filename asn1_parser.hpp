// ----------------------------------------------------------------------------
// Copyright (C) 2015-2016 zunceng@gmail.com
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
// For more information, see www.boost.org
// ----------------------------------------------------------------------------
#ifndef BOOST_PROPERTY_TREE_ASN1_PARSER_HPP_INCLUDED
#define BOOST_PROPERTY_TREE_ASN1_PARSER_HPP_INCLUDED

#include <boost/property_tree/ptree.hpp>
#include "detail/asn1_parser_read.hpp"
#include "detail/asn1_parser_write.hpp"
#include "detail/asn1_parser_error.hpp"
#include "detail/tap3_parser_read.hpp"

#include <fstream>
#include <string>
#include <locale>


namespace boost { namespace property_tree { namespace asn1_parser
{
    template<class Ptree>
    void read_asn1(std::basic_istream<
                       typename Ptree::key_type::value_type
                   > &stream,
                   Ptree &pt)
    {
        read_asn1_internal(stream, pt, std::string());
    }
    
    template<class Ptree>
    void read_asn1(const std::string &filename,
                   Ptree &pt,
                   const std::locale &loc = std::locale())
    {
        std::basic_ifstream<typename Ptree::key_type::value_type>
            stream(filename.c_str());
        if (!stream)
            BOOST_PROPERTY_TREE_THROW(asn1_parser_error(
                "cannot open file", filename, 0));
        stream.imbue(loc);
        read_asn1_internal(stream, pt, filename);
    }
/*
    template<class Ptree>
    void write_asn1(std::basic_ostream<
                        typename Ptree::key_type::value_type
                    > &stream,
                    const Ptree &pt,
                    bool pretty = true)
    {
        write_asn1_internal(stream, pt, std::string(), pretty);
    }

    template<class Ptree>
    void write_asn1(const std::string &filename,
                    const Ptree &pt,
                    const std::locale &loc = std::locale(),
                    bool pretty = true)
    {
        std::basic_ofstream<typename Ptree::key_type::value_type>
            stream(filename.c_str());
        if (!stream)
            BOOST_PROPERTY_TREE_THROW(asn1_parser_error(
                "cannot open file", filename, 0));
        stream.imbue(loc);
        write_asn1_internal(stream, pt, filename, pretty);
    }
*/  

namespace tap_parser{
    using namespace boost::property_tree::detail::tap_parser;    
}

    
} } }

#endif