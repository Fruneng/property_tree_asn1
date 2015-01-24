// ----------------------------------------------------------------------------
// Copyright (C) 2015-2016 zunceng@gmail.com
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
// For more information, see www.boost.org
// ----------------------------------------------------------------------------
#ifndef BOOST_PROPERTY_TREE_TAP3_PARSER_READ_HPP_INCLUDED
#define BOOST_PROPERTY_TREE_TAP3_PARSER_READ_HPP_INCLUDED

#include <boost/property_tree/ptree.hpp>
#include "asn1_parser_read.hpp"
#include <boost/lexical_cast.hpp>
#include <set>

namespace boost { namespace property_tree { namespace detail {namespace tap_parser{

    enum tap_type 
    {
        Group,
        Integer,
        Integer64,
        OctString,
        BcdString,
    };

    typedef struct tap_element_t{
        std::string     name;
        std::size_t     tag;
        tap_type        type;
        
        bool operator < (const struct tap_element_t &rhs) const
        {
            return tag<rhs.tag;
        }
    } tap_element;
    
    namespace internal
    {
        template <int Version, int Release>
        struct lookup_tables
        {
            static const tap_element tap_elements[1024];
        };
    }
    
    void trans_asn1_ptree_internal(
        boost::property_tree::ptree &pt, 
        boost::property_tree::ptree &new_pt, 
        const std::set<tap_element>& tap3_lookup_map)
    {
        boost::property_tree::ptree::assoc_iterator it = pt.ordered_begin();
        for (;it!=pt.not_found(); it++)
        {
            std::size_t tag = boost::lexical_cast<std::size_t>(it->first);
            tap_element tmp = {"", tag, Group};
            
            std::set<tap_element>::const_iterator lookup_it = tap3_lookup_map.find(tmp);
            if (lookup_it != tap3_lookup_map.end())
            {
                boost::property_tree::ptree &new_node = new_pt.push_back(
                    std::make_pair(lookup_it->name, boost::property_tree::ptree()))->second;
                
                switch(lookup_it->type)
                {
                    case Integer:
                    case Integer64:
                    {
                        long long tmp = boost::property_tree::asn1_parser::binary2Int<0>((it->second).data());
                        new_node.data() = boost::lexical_cast<std::string>(tmp);
                    }break;
                    case OctString:
                    {
                        new_node.data() = boost::property_tree::asn1_parser::binary2OCTString<0>((it->second).data());
                    }break;
                    case BcdString:
                    {
                        new_node.data() = boost::property_tree::asn1_parser::binary2BCDString<0>((it->second).data());
                    }break;
                    default:
                        break;
                }
                
                trans_asn1_ptree_internal(it->second, new_node, tap3_lookup_map);
            }
            
           
        }
    }

    template<int Version, int Release>
    void trans_asn1_ptree(boost::property_tree::ptree &pt, boost::property_tree::ptree& new_pt)
    {
        static const std::set<tap_element> tap3_lookup_map(
            &(internal::lookup_tables<3,11>::tap_elements[0]), 
            &(internal::lookup_tables<3,11>::tap_elements[0]) + sizeof(internal::lookup_tables<3,11>::tap_elements)/sizeof(tap_element));
    
        
        trans_asn1_ptree_internal(pt, new_pt, tap3_lookup_map);
    }
    
    namespace internal
    {    
        template <int Version, int Release>
        const tap_element lookup_tables<Version, Release>::tap_elements[] = {};

        template <>
        const tap_element lookup_tables<3, 11>::tap_elements[] = {
            {"TransferBatch" ,1, Group},
            {"Notification" ,2, Group},
            {"CallEventDetailList" ,3, Group},
            {"BatchControlInfo" ,4, Group},
            {"AccountingInfo" ,5, Group},
            {"NetworkInfo" ,6, Group},
            {"MessageDescriptionInfoList" ,8, Group},
            {"MobileOriginatedCall" ,9, Group},
            {"MobileTerminatedCall" ,10, Group},
            {"SupplServiceEvent" ,11, Group},
            {"ServiceCentreUsage" ,12, Group},
            {"GprsCall" ,14, Group},
            {"ContentTransaction" ,17, Group},
            {"LocationService" ,297, Group},
            {"AuditControlInfo" ,15, Group},
            {"AccessPointNameNI" ,261, OctString},
            {"AccessPointNameOI" ,262, OctString},
            {"ActualDeliveryTimeStamp" ,302, Group},
            {"AdvisedCharge" ,349, Group},
            {"AdvisedChargeCurrency" ,348, Group},
            {"AdvisedChargeInformation" ,351, Group},
            {"AgeOfLocation" ,396, Integer},
            {"BasicService" ,36, Group},
            {"BasicServiceCode" ,426, Group },
            {"BasicServiceCodeList" ,37, Group},
            {"BasicServiceUsed" ,39, Group},
            {"BasicServiceUsedList" ,38, Group},
            {"BearerServiceCode" ,40, OctString},
            {"CalledNumber" ,407, Integer},
            {"CalledPlace" ,42, OctString},
            {"CalledRegion" ,46, OctString},
            {"CallEventDetailsCount" ,43, Integer },
            {"CallEventStartTimeStamp" ,44, Group},
            {"CallingNumber" ,405, Integer},
            {"CallOriginator" ,41,  Group},
            {"CallReference" ,45, Integer},
            {"CallTypeGroup" ,258, Group},
            {"CallTypeLevel1" ,259, Integer},
            {"CallTypeLevel2" ,255, Integer},
            {"CallTypeLevel3" ,256, Integer},
            {"CamelDestinationNumber" ,404, OctString},
            {"CamelInvocationFee" ,422, Integer},
            {"CamelServiceKey" ,55, Integer},
            {"CamelServiceLevel" ,56, Integer},
            {"CamelServiceUsed" ,57, Group},
            {"CauseForTerm" ,58, Integer},
            {"CellId" ,59, Integer },
            {"Charge" ,62, Integer},
            {"ChargeableSubscriber" ,427, Group },
            {"ChargeableUnits" ,65,  Integer},
            {"ChargeDetail" ,63, Group},
            {"ChargeDetailList" ,64, Group},
            {"ChargeDetailTimeStamp" ,410, Group},
            {"ChargedItem" ,66,  OctString},
            {"ChargedPartyEquipment" ,323, Group},
            {"ChargedPartyHomeIdentification" ,313, Group},
            {"ChargedPartyHomeIdList" ,314, Group },
            {"ChargedPartyIdentification" ,309, Group},
            {"ChargedPartyIdentifier" ,287, OctString},
            {"ChargedPartyIdList" ,310, Group},
            {"ChargedPartyIdType" ,305, Integer},
            {"ChargedPartyInformation" ,324, Group},
            {"ChargedPartyLocation" ,320, Group},
            {"ChargedPartyLocationList" ,321, Group },
            {"ChargedPartyStatus" ,67, Integer },
            {"ChargedUnits" ,68,  Integer },
            {"ChargeInformation" ,69, Group},
            {"ChargeInformationList" ,70, Group},
            {"ChargeRefundIndicator" ,344, Integer},
            {"ChargeType" ,71, OctString },
            {"ChargingId" ,72, Integer},
            {"ChargingPoint" ,73,  OctString},
            {"ChargingTimeStamp" ,74,  Group},
            {"ClirIndicator" ,75, Integer},
            {"Commission" ,350, Group},
            {"CompletionTimeStamp" ,76, Group},
            {"ContentChargingPoint" ,345, Integer},
            {"ContentProvider" ,327, Group},
            {"ContentProviderIdentifier" ,292, OctString},
            {"ContentProviderIdList" ,328, Group },
            {"ContentProviderIdType" ,291, Integer},
            {"ContentProviderName" ,334, OctString},
            {"ContentServiceUsed" ,352, Group},
            {"ContentServiceUsedList" ,285, Group},
            {"ContentTransactionBasicInfo" ,304, Group},
            {"ContentTransactionCode" ,336, Integer },
            {"ContentTransactionType" ,337, Integer },
            {"CseInformation" ,79, OctString},
            {"CurrencyConversion" ,106, Group},
            {"CurrencyConversionList" ,80, Group },
            {"CustomerIdentifier" ,364, OctString},
            {"CustomerIdType" ,363, Integer},
            {"DataVolumeIncoming" ,250, Integer64},
            {"DataVolumeOutgoing" ,251, Integer64},
            {"DefaultCallHandlingIndicator" ,87, Integer},
            {"DepositTimeStamp" ,88, Group},
            {"Destination" ,89, Group},
            {"DestinationNetwork" ,90, OctString },
            {"DialledDigits" ,279, OctString},
            {"Discount" ,412, Integer},
            {"DiscountableAmount" ,423, Integer},
            {"DiscountApplied" ,428, Group },
            {"DiscountCode" ,91, Integer},
            {"DiscountInformation" ,96, Group},
            {"Discounting" ,94, Group},
            {"DiscountingList" ,95,  Group},
            {"DiscountRate" ,92, Integer},
            {"DistanceChargeBandCode" ,98, OctString },
            {"EarliestCallTimeStamp" ,101, Group},
            {"EquipmentId" ,290, OctString},
            {"EquipmentIdType" ,322, Integer},
            {"Esn" ,103, OctString},
            {"ExchangeRate" ,104, Integer},
            {"ExchangeRateCode" ,105, Integer},
            {"FileAvailableTimeStamp" ,107, Group},
            {"FileCreationTimeStamp" ,108, Group},
            {"FileSequenceNumber" ,109, OctString},
            {"FileTypeIndicator" ,110, OctString},
            {"FixedDiscountValue" ,411, Integer},
            {"Fnur" ,111, Integer},
            {"GeographicalLocation" ,113,  Group},
            {"GprsBasicCallInformation" ,114, Group},
            {"GprsChargeableSubscriber" ,115, Group},
            {"GprsDestination" ,116, Group},
            {"GprsLocationInformation" ,117, Group},
            {"GprsNetworkLocation" ,118, Group},
            {"GprsServiceUsed" ,121,  Group},
            {"GsmChargeableSubscriber" ,286, Group},
            {"GuaranteedBitRate" ,420, OctString },
            {"HomeBid" ,122,  OctString},
            {"HomeIdentifier" ,288, OctString},
            {"HomeIdType" ,311, Integer},
            {"HomeLocationDescription" ,413, OctString},
            {"HomeLocationInformation" ,123, Group},
            {"HorizontalAccuracyDelivered" ,392, Integer},
            {"HorizontalAccuracyRequested" ,385, Integer},
            {"HSCSDIndicator" ,424, OctString },
            {"Imei" ,128, BcdString },
            {"ImeiOrEsn" ,429, Group },
            {"Imsi" ,129, BcdString},
            {"IMSSignallingContext" ,418, Integer},
            {"InternetServiceProvider" ,329, Group},
            {"InternetServiceProviderIdList" ,330, Group},
            {"IspIdentifier" ,294, OctString},
            {"IspIdType" ,293, Integer},
            {"ISPList" ,378, Group },
            {"NetworkIdType" ,331, Integer},
            {"NetworkIdentifier" ,295, OctString},
            {"Network" ,332, Group },
            {"NetworkList" ,333, Group},
            {"LatestCallTimeStamp" ,133, Group},
            {"LCSQosDelivered" ,390, Group},
            {"LCSQosRequested" ,383, Group},
            {"LCSRequestTimestamp" ,384, Group},
            {"LCSSPIdentification" ,375, Group},
            {"LCSSPIdentificationList" ,374, Group},
            {"LCSSPInformation" ,373, Group},
            {"LCSTransactionStatus" ,391, Integer},
            {"LocalCurrency" ,135, OctString},
            {"LocalTimeStamp" ,16, OctString},
            {"LocationArea" ,136, Integer },
            {"LocationIdentifier" ,289, OctString},
            {"LocationIdType" ,315, Integer},
            {"LocationInformation" ,138,  Group},
            {"LocationServiceUsage" ,382, Group},
            {"MaximumBitRate" ,421, OctString },
            {"Mdn" ,253, OctString},
            {"MessageDescription" ,142, OctString},
            {"MessageDescriptionCode" ,141, Integer},
            {"MessageDescriptionInformation" ,143, Group},
            {"MessageStatus" ,144, Integer},
            {"MessageType" ,145, Integer},
            {"Min" ,146, OctString },
            {"MinChargeableSubscriber" ,254, Group},
            {"MoBasicCallInformation" ,147, Group},
            {"Msisdn" ,152, Integer },
            {"MtBasicCallInformation" ,153, Group},
            {"NetworkAccessIdentifier" ,417, OctString},
            {"NetworkInitPDPContext" ,245, Integer},
            {"NetworkLocation" ,156,  Group},
            {"NonChargedNumber" ,402, OctString},
            {"NumberOfDecimalPlaces" ,159, Integer},
            {"ObjectType" ,281, Integer},
            {"OperatorSpecInfoList" ,162, Group },
            {"OperatorSpecInformation" ,163, OctString},
            {"OrderPlacedTimeStamp" ,300, Group},
            {"OriginatingNetwork" ,164, OctString },
            {"PacketDataProtocolAddress" ,165, OctString },
            {"PaidIndicator" ,346, Integer},
            {"PartialTypeIndicator " ,166, OctString },
            {"PaymentMethod" ,347, Integer},
            {"PdpAddress" ,167, OctString},
            {"PDPContextStartTimestamp" ,260, Group},
            {"PlmnId" ,169, OctString },
            {"PositioningMethod" ,395, Integer},
            {"PriorityCode" ,170, Integer},
            {"RapFileSequenceNumber" ,181,  OctString},
            {"RecEntityCode" ,184, Integer},
            {"RecEntityCodeList" ,185, Group },
            {"RecEntityId" ,400, OctString},
            {"RecEntityInfoList" ,188, Group},
            {"RecEntityInformation" ,183, Group},
            {"RecEntityType" ,186, Integer},
            {"Recipient" ,182,  OctString},
            {"ReleaseVersionNumber" ,189, Integer},
            {"RequestedDeliveryTimeStamp" ,301, Group},
            {"ResponseTime" ,394, Integer},
            {"ResponseTimeCategory" ,387, Integer},
            {"ScuBasicInformation" ,191, Group},
            {"ScuChargeType" ,192,  Group},
            {"ScuTimeStamps" ,193,  Group},
            {"ScuChargeableSubscriber" ,430, Group },
            {"Sender" ,196,  OctString},
            {"ServingBid" ,198,  OctString},
            {"ServingLocationDescription" ,414, OctString},
            {"ServingNetwork" ,195,  OctString},
            {"ServingPartiesInformation" ,335, Group},
            {"SimChargeableSubscriber" ,199, Group},
            {"SimToolkitIndicator" ,200, OctString},
            {"SMSDestinationNumber" ,419, OctString},
            {"SMSOriginator" ,425, OctString},
            {"SpecificationVersionNumber " ,201, Integer},
            {"SsParameters" ,204, OctString},
            {"SupplServiceActionCode" ,208, Integer},
            {"SupplServiceCode" ,209, OctString},
            {"SupplServiceUsed" ,206, Group},
            {"TapCurrency" ,210, OctString},
            {"TapDecimalPlaces" ,244, Integer},
            {"TaxableAmount" ,398, Integer},
            {"Taxation" ,216, Group},
            {"TaxationList" ,211,  Group},
            {"TaxCode" ,212, Integer},
            {"TaxInformation" ,213, Group},
            {"TaxInformationList" ,214,  Group},
            {"TaxRate" ,215, OctString },
            {"TaxType" ,217, OctString },
            {"TaxValue" ,397, Integer},
            {"TeleServiceCode" ,218, OctString},
            {"ThirdPartyInformation" ,219,  Group},
            {"ThirdPartyNumber" ,403, OctString},
            {"ThreeGcamelDestination" ,431, Group},
            {"TotalAdvisedCharge" ,356, Integer },
            {"TotalAdvisedChargeRefund" ,357, Integer},
            {"TotalAdvisedChargeValue" ,360, Group},
            {"TotalAdvisedChargeValueList" ,361, Group },
            {"TotalCallEventDuration" ,223, Integer },
            {"TotalCharge" ,415, Integer},
            {"TotalChargeRefund" ,355, Integer},
            {"TotalCommission" ,358, Integer},
            {"TotalCommissionRefund" ,359, Integer},
            {"TotalDataVolume" ,343, Integer64},
            {"TotalDiscountRefund" ,354, Integer},
            {"TotalDiscountValue" ,225, Integer},
            {"TotalTaxRefund" ,353, Integer},
            {"TotalTaxValue" ,226, Integer},
            {"TotalTransactionDuration" ,416, Integer64},
            {"TrackedCustomerEquipment" ,381, Group},
            {"TrackedCustomerHomeId" ,377, Group},
            {"TrackedCustomerHomeIdList" ,376, Group },
            {"TrackedCustomerIdentification" ,372, Group},
            {"TrackedCustomerIdList" ,370, Group },
            {"TrackedCustomerInformation" ,367, Group},
            {"TrackedCustomerLocation" ,380, Group},
            {"TrackedCustomerLocList" ,379, Group },
            {"TrackingCustomerEquipment" ,371, Group},
            {"TrackingCustomerHomeId" ,366, Group},
            {"TrackingCustomerHomeIdList" ,365, Group },
            {"TrackingCustomerIdentification" ,362, Group},
            {"TrackingCustomerIdList" ,299, Group },
            {"TrackingCustomerInformation" ,298, Group},
            {"TrackingCustomerLocation" ,369, Group},
            {"TrackingCustomerLocList" ,368, Group },
            {"TrackingFrequency" ,389, Integer},
            {"TrackingPeriod" ,388, Integer},
            {"TransactionAuthCode" ,342, OctString},
            {"TransactionDescriptionSupp" ,338, Integer},
            {"TransactionDetailDescription" ,339, OctString},
            {"TransactionIdentifier" ,341, OctString},
            {"TransactionShortDescription" ,340, OctString},
            {"TransactionStatus" ,303, Integer},
            {"TransferCutOffTimeStamp" ,227, Group},
            {"TransparencyIndicator" ,228, Integer},
            {"UserProtocolIndicator" ,280, Integer},
            {"UtcTimeOffset" ,231, OctString },
            {"UtcTimeOffsetCode" ,232, Integer},
            {"UtcTimeOffsetInfo" ,233, Group},
            {"UtcTimeOffsetInfoList" ,234,  Group },
            {"VerticalAccuracyDelivered" ,393, Integer},
            {"VerticalAccuracyRequested" ,386, Integer},  
        };

/*
        template <>
        const tap_element lookup_tables<3, 12>::tap_elements[1024] = {
       
        };
        
        template <>
        const tap_element lookup_tables<3, 10>::tap_elements[1024] = {
       
        };
        
        template <>
        const tap_element lookup_tables<3, 9>::tap_elements[1024] = {
       
        };
        
        template <>
        const tap_element lookup_tables<3, 3>::tap_elements[1024] = {
       
        };
*/
    }

}}}}

#endif
