#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include "asn1_parser.hpp"
#include "detail/rapidasn1.hpp"
#include <fstream>
#include <iostream>
#include <assert.h>
typedef unsigned char Byte;


// Loads debug_settings structure from the specified XML file
void load(const std::string &filename)
{
    // Create an empty property tree object
    using boost::property_tree::ptree;
    ptree pt;
    read_xml(filename, pt);
    std::cout << pt.get<std::string>("debug.filename") << std::endl;
    std::cout << pt.get("debug.level", 0) << std::endl;
}
// Loads debug_settings structure from the specified asn1 file
void load2(const std::string &filename)
{
    // Create an empty property tree object
    using boost::property_tree::ptree;
    using namespace boost::property_tree::asn1_parser;
    
    boost::property_tree::ptree pt;
    boost::property_tree::asn1_parser::read_asn1(filename, pt);
    
    // print_ptree(pt);
    // std::cout << pt.get<std::string>("1.4.196") << std::endl;
    boost::property_tree::ptree new_pt;
    
    boost::property_tree::asn1_parser::tap_parser::trans_asn1_ptree<3, 11>(pt, new_pt);
    
    write_xml(filename+".xml", new_pt);
    // std::cout << pt.get<std::string>("15.101.231") << std::endl;
        
}

void test_rapodasn1_parse_tag(unsigned char* buff, std::size_t size, size_t tag)
{
    // char buff[] = {0x5f, 0x81, 0x44};
    std::string v(buff,buff+size);
    boost::property_tree::detail::rapidasn1::asn1_tree<Byte> tree;
    try{
        boost::property_tree::detail::rapidasn1::asn1_node<Byte> node(
            boost::property_tree::detail::rapidasn1::node_nongroup);
        tree.parse_tag<1>((const Byte*)v.c_str(), v.length(), &node);
        assert(node.tag() == tag);
        // assert(node.tag() == 196);
    }
    catch(boost::property_tree::detail::rapidasn1::parse_error &e)
    {
        std::cout << e.what() << std::endl;
        std::cout << e.where() << std::endl;
    }
}

void test_rapodasn1_parse_len()
{
    // char buff[] = {0x05};
    unsigned char buff[] = {0x82, 0xEA, 0xEF};
    std::string v(buff,buff+sizeof(buff));
    boost::property_tree::detail::rapidasn1::asn1_tree<Byte> tree;
    try{
        boost::property_tree::detail::rapidasn1::asn1_node<Byte> node(
            boost::property_tree::detail::rapidasn1::node_nongroup);
        int is_varlen;
        tree.parse_len<1>((const Byte*)v.c_str(), v.length(), &node, is_varlen);
        assert(node.value_size() == 60143);
        assert(is_varlen == 0);
    }
    catch(boost::property_tree::detail::rapidasn1::parse_error &e)
    {
        std::cout << e.what() << std::endl;
        std::cout << e.where() << std::endl;
    }
}

void test_rapodasn1_parse()
{
    unsigned char buff[] = {0x5F ,0x81, 0x44, 0x05, 0x41, 0x55, 0x54, 0x4D, 0x4D};
    std::string v(buff,buff+sizeof(buff));
    boost::property_tree::detail::rapidasn1::asn1_tree<Byte> tree;
    try{
        boost::property_tree::detail::rapidasn1::asn1_node<Byte> node(
            boost::property_tree::detail::rapidasn1::node_nongroup);
        tree.parse<1>((const Byte*)v.c_str(), v.length());
    }   
    catch(boost::property_tree::detail::rapidasn1::parse_error &e)
    {
        std::cout << e.what() << std::endl;
        std::cout << e.where() << std::endl;
    }
}

void test_binary2Int(std::string data, long long real)
{    
    long long ret = boost::property_tree::asn1_parser::binary2Int<0>(data);
    assert(real == ret);
}


void test_rapidasn1()
{
    unsigned char buff1[] = {0x5f, 0x81, 0x44};
    size_t tag1 = 196;
    test_rapodasn1_parse_tag(buff1, sizeof(buff1), tag1);
    
    unsigned char buff2[] = {0x7f, 0x81, 0x63};
    size_t tag2 = 227;
    test_rapodasn1_parse_tag(buff2, sizeof(buff2), tag2);
    
    test_rapodasn1_parse_len();
    test_rapodasn1_parse();
    
    unsigned char buff3[] = {0x5B, 0xC2};
    std::string v(buff3, buff3+sizeof(buff3));
    test_binary2Int(v, 23490);
    
}

void test_asn1file()
{
    try{
        load2("CDAFGAWDNKDM05958");
    }   
    catch(boost::property_tree::detail::rapidasn1::parse_error &e)
    {
        std::cout << e.what() << std::endl;
        std::cout << e.where() << std::endl;
    }
}

int main()
{
    // load("test.xml");
    load2("CDAFGAWDNKDM05958");
    
    // test_asn1file();
    
    //test_rapidasn1();
    

}
