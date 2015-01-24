// ----------------------------------------------------------------------------
// Copyright (C) 2014-20?? zunceng@gmail.com
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
// For more information, see www.boost.org
// ----------------------------------------------------------------------------
#ifndef BOOST_PROPERTY_TREE_DETAIL_ASN1_PARSER_READ_HPP_INCLUDED
#define BOOST_PROPERTY_TREE_DETAIL_ASN1_PARSER_READ_HPP_INCLUDED

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/detail/ptree_utils.hpp>
#include <boost/spirit/include/classic.hpp>
#include <boost/limits.hpp>
#include <string>
#include <locale>
#include <istream>
#include <vector>
#include <algorithm>
#include <boost/lexical_cast.hpp>
#include "asn1_parser_error.hpp"
#include "rapidasn1.hpp"


namespace boost { namespace property_tree { namespace asn1_parser
{
    template<class Ptree, class Byte>
    void read_asn1_node(detail::rapidasn1::asn1_node<Byte> *node,
                       Ptree &pt)
    {
        using namespace detail::rapidasn1;
        
        if (!node)
            return;
        
        switch (node->type())
        {
            case node_nongroup:
            {
                pt.data() = typename Ptree::key_type((const char*)node->value(), node->value_size());
            }break;
            case node_group:
            {
                // Copy children
                for (asn1_node<Byte> *child = node->first_node(); child; child = child->next_sibling())
                {
                    Ptree &pt_node = pt.push_back(
                        std::make_pair(boost::lexical_cast<std::string>(child->tag()),Ptree()))->second;
                    read_asn1_node(child, pt_node);
                }
            }break;
            default:
                // Skip other node types
                break;
        }
    }
    
    template<class Ptree>
    void read_asn1_internal(std::basic_istream<typename Ptree::key_type::value_type> &stream,
                            Ptree &pt,
                            const std::string &filename)
    {
        typedef typename Ptree::key_type::value_type Ch;
        typedef unsigned char Byte;

        // Load data into vector
        std::vector<Ch> v(std::istreambuf_iterator<Ch>(stream.rdbuf()),
                          std::istreambuf_iterator<Ch>());
        if (!stream.good())
            BOOST_PROPERTY_TREE_THROW(asn1_parser_error("read error", filename, 0));

        boost::property_tree::detail::rapidasn1::asn1_tree<Byte> tree;
        tree.parse<1>((const Byte*)(&*v.begin()), v.size());
        
        // tree.print<1>();
        read_asn1_node(&tree, pt);
    }
    
    template<int Flags>
    long long binary2Int(const std::string& data)
    {
        if (data.length() <= 8&& data.length()>0)
        {
            unsigned long long ret = 0;
            
            int postive = (*data.begin()) & 0x80;
            for(std::string::const_iterator it=data.begin(); it!=data.end(); it++)
            {
                ret <<= 8;
                ret |= reinterpret_cast<const unsigned char&>(*it);
            }
            if (postive)
            {
                for(int i = data.length(); i<8; i++)
                {
                    ret |= (0xFFULL) << (i*8);
                }
            }
            return reinterpret_cast<long long&>(ret);
        }
        BOOST_PROPERTY_TREE_THROW(asn1_parser_error("parse int error", "", 0));
        return 0;
    }

    template<int Flags>
    std::string binary2OCTString(const std::string& data)
    {
        return data;
    }
    template<int Flags>
    std::string binary2BCDString(const std::string& data)
    {
        std::string ret;
        for(std::string::const_iterator it=data.begin(); it!=data.end(); it++)
        {
            char high = ((*it)&0xF0)>>4;
            char low = (*it)&0x0F;
            
            if (high == 0x0F)
                break;
            ret.append(1, high+'0');
            if (low == 0x0F)
                break;
            ret.append(1, low+'0');
        }
        return ret;
    }
    
} } }

#endif
