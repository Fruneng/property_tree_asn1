# property_tree_asn1
boost property tree addon library for asn1(TAP3)

asn1 file like a "binary" xml.

support this operator like:

  using boost::property_tree::ptree;
  using namespace boost::property_tree::asn1_parser;
  
  boost::property_tree::ptree pt;
  boost::property_tree::asn1_parser::read_asn1(filename, pt);
  
  boost::property_tree::ptree new_pt;
  
  boost::property_tree::asn1_parser::tap_parser::trans_asn1_ptree<3, 11>(pt, new_pt);
  
  std::cout << new_pt.get<std::string>("TransferBatch.BatchControlInfo.Sender") << std::endl;
  std::cout << new_pt.get<std::string>("TransferBatch.BatchControlInfo.Recipient ") << std::endl;
