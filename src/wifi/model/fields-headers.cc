/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2015 HANY ASSASA
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Hany Assasa <hany.assasa@hotmail.com>
 */
#include "ns3/fatal-error.h"
#include "ns3/log.h"

#include "fields-headers.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("FieldsHeaders");

/***********************************
 * Sector Sweep (SSW) Field (8.4a.1)
 ***********************************/

NS_OBJECT_ENSURE_REGISTERED (DMG_SSW_Field);

DMG_SSW_Field::DMG_SSW_Field()
  : m_dir(BeamformingInitiator),
    m_cdown(0),
    m_sid(0),
    m_antenna_id(0),
    m_length(0)
{
    NS_LOG_FUNCTION(this);
}

DMG_SSW_Field::~DMG_SSW_Field()
{
    NS_LOG_FUNCTION(this);
}

TypeId DMG_SSW_Field::GetTypeId(void)
{
    NS_LOG_FUNCTION_NOARGS();
    static TypeId tid = TypeId("ns3::DMG_SSW_Field")
	.SetParent<Header>()
	.AddConstructor<DMG_SSW_Field>()
	;
    return tid;
}

TypeId DMG_SSW_Field::GetInstanceTypeId(void) const
{
    NS_LOG_FUNCTION(this);
    return GetTypeId();
}

void DMG_SSW_Field::Print(std::ostream &os) const
{
    NS_LOG_FUNCTION(this << &os);
    os << "Direction=" << m_dir
       << ", CDOWN=" << m_cdown
       << ", SID=" << m_sid
       << ", Antenna ID=" << m_antenna_id
       << ", RXSS Length=" << m_length;
}

uint32_t DMG_SSW_Field::GetSerializedSize() const
{
    NS_LOG_FUNCTION(this);
    return 3;
}

void DMG_SSW_Field::Serialize(Buffer::Iterator start) const
{
    NS_LOG_FUNCTION (this << &start);
    Buffer::Iterator i = start;
    uint8_t ssw[3];

    ssw[0] |= m_dir & 0x1;
    ssw[0] |= ((m_cdown & 0x7F) << 1);
    ssw[1] |= ((m_cdown & 0x180) >> 7);
    ssw[1] |= ((m_sid & 0x3F) << 2);
    ssw[2] |= m_antenna_id & 0x3;
    ssw[2] |= ((m_length & 0x3F) << 2);

    i.Write(ssw, 3);
}

uint32_t DMG_SSW_Field::Deserialize(Buffer::Iterator start)
{
    NS_LOG_FUNCTION(this << &start);
    Buffer::Iterator i = start;
    uint8_t ssw[3];

    i.Read(ssw, 3);
    m_dir = static_cast<enum Direction> (ssw[0] & 0x1);
    m_cdown = (static_cast<uint16_t>(ssw[0] & 0xFE) >> 1) &
	      (static_cast<uint16_t>(ssw[1] & 0x03) << 7);
    m_sid = (ssw[1] & 0xFC) >> 2;
    m_antenna_id = ssw[2] & 0x3;
    m_length = (ssw[2] & 0xFC) >> 2;

    return i.GetDistanceFrom(start);
}

void DMG_SSW_Field::SetDirection(enum Direction dir)
{
    NS_LOG_FUNCTION(this << dir);
    m_dir = dir;
}

void DMG_SSW_Field::SetCountDown(uint16_t cdown)
{
    NS_LOG_FUNCTION(this << cdown);
    m_cdown = cdown;
}

void DMG_SSW_Field::SetSectorID(uint8_t sid)
{
    NS_LOG_FUNCTION(this << sid);
    m_sid = sid;
}

void DMG_SSW_Field::SetDMGAntennaID(uint8_t antenna_id)
{
    NS_LOG_FUNCTION(this << antenna_id);
    m_antenna_id = antenna_id;
}

void DMG_SSW_Field::SetRXSSLength(uint8_t length)
{
    NS_LOG_FUNCTION(this << length);
    m_length = length;
}

enum Direction DMG_SSW_Field::GetDirection(void) const
{
    NS_LOG_FUNCTION(this);
    return m_dir;
}

uint16_t DMG_SSW_Field::GetCountDown(void) const
{
    NS_LOG_FUNCTION(this);
    return m_cdown;
}

uint8_t DMG_SSW_Field::GetSectorID(void) const
{
    NS_LOG_FUNCTION(this);
    return m_sid;
}

uint8_t DMG_SSW_Field::GetDMGAntennaID(void) const
{
    NS_LOG_FUNCTION(this);
    return m_antenna_id;
}

uint8_t DMG_SSW_Field::GetRXSSLength(void) const
{
    NS_LOG_FUNCTION(this);
    return m_length;
}

/*****************************************
 * Dynamic Allocation Info Field (8.4a.2)
 *****************************************/

NS_OBJECT_ENSURE_REGISTERED (Dynamic_Allocation_Info_Field);

Dynamic_Allocation_Info_Field::Dynamic_Allocation_Info_Field()
  : m_tid(0),
    m_allocationType(SP_Allocation),
    m_source_AID(0),
    m_destination_AID(0),
    m_allocation_duration(0)
{
    NS_LOG_FUNCTION(this);
}

Dynamic_Allocation_Info_Field::~Dynamic_Allocation_Info_Field()
{
    NS_LOG_FUNCTION(this);
}

TypeId Dynamic_Allocation_Info_Field::GetTypeId(void)
{
    NS_LOG_FUNCTION_NOARGS();
    static TypeId tid = TypeId("ns3::Dynamic_Allocation_Info_Field")
	.SetParent<Header>()
	.AddConstructor<Dynamic_Allocation_Info_Field>()
	;
    return tid;
}

TypeId Dynamic_Allocation_Info_Field::GetInstanceTypeId(void) const
{
    NS_LOG_FUNCTION(this);
    return GetTypeId();
}

void Dynamic_Allocation_Info_Field::Print(std::ostream &os) const
{
    NS_LOG_FUNCTION(this << &os);
}

uint32_t Dynamic_Allocation_Info_Field::GetSerializedSize() const
{
    NS_LOG_FUNCTION(this);
    return 5;
}

void Dynamic_Allocation_Info_Field::Serialize(Buffer::Iterator start) const
{
    NS_LOG_FUNCTION (this << &start);
    Buffer::Iterator i = start;
    uint32_t field1 = 0;
    uint8_t field2 = 0;

    field1 |= m_tid & 0xF;
    field1 |= (m_allocationType & 0x7) << 4;
    field1 |= m_source_AID << 7;
    field1 |= m_destination_AID << 15;
    field1 |= (m_allocation_duration & 0x1FF) << 23;

    field2 |= (m_allocation_duration >> 7) & 0x7F;
    field2 |= (m_reserved & 0x1) << 7;

    i.WriteHtolsbU32(field1);
    i.WriteU8(field2);
}

uint32_t Dynamic_Allocation_Info_Field::Deserialize(Buffer::Iterator start)
{
    NS_LOG_FUNCTION(this << &start);
    Buffer::Iterator i = start;
    uint32_t field1 = i.ReadLsbtohU32();
    uint8_t field2 = i.ReadU8();

    m_tid = field1 & 0xF;
    m_allocationType = static_cast<enum AllocationType> ((m_allocationType >> 4) & 0x7);
    m_source_AID = (field1 >> 7) & 0xFF;
    m_destination_AID = (field1 >> 7) & 0xFF;
    m_allocation_duration = (static_cast<uint16_t>(field1 >> 23) & 0x1FF) |
			    (static_cast<uint16_t>(field2 & 0x7F) << 7);
    m_reserved = (field2 >> 7) & 0x1;

    return i.GetDistanceFrom(start);
}

/*********************************************
 * Sector Sweep Feedback (SSW) Field (8.4a.3)
 *********************************************/

NS_OBJECT_ENSURE_REGISTERED(DMG_SSW_FBCK_Field);

DMG_SSW_FBCK_Field::DMG_SSW_FBCK_Field()
  : m_sectors(0),
    m_antennas(0),
    m_snr_report(0),
    m_poll_required(false),
    m_reserved(0),
    m_is_iss(false)
{
    NS_LOG_FUNCTION(this);
}

DMG_SSW_FBCK_Field::~DMG_SSW_FBCK_Field()
{
    NS_LOG_FUNCTION(this);
}

TypeId DMG_SSW_FBCK_Field::GetTypeId(void)
{
    NS_LOG_FUNCTION_NOARGS();
    static TypeId tid = TypeId("ns3::DMG_SSW_FBCK_Field")
	.SetParent<Header>()
	.AddConstructor<DMG_SSW_FBCK_Field>()
	;
    return tid;
}

TypeId DMG_SSW_FBCK_Field::GetInstanceTypeId(void) const
{
    NS_LOG_FUNCTION(this);
    return GetTypeId();
}

void DMG_SSW_FBCK_Field::Print(std::ostream &os) const
{
    NS_LOG_FUNCTION(this << &os);
    if (m_is_iss)
    {

    }
    else
    {

    }
}

uint32_t DMG_SSW_FBCK_Field::GetSerializedSize() const
{
    NS_LOG_FUNCTION(this);
    return 3;
}

void DMG_SSW_FBCK_Field::Serialize(Buffer::Iterator start) const
{
    NS_LOG_FUNCTION(this << &start);
    Buffer::Iterator i = start;
    uint8_t ssw[3];
    memset(ssw, 0, sizeof(ssw));

    if (m_is_iss)
    {
	ssw[0] |= m_sectors & 0xFF;
	ssw[1] |= (m_sectors >> 8) & 0x1;
	ssw[1] |= (m_antennas & 0x3) << 1;
	ssw[1] |= (m_snr_report & 0x1F) << 3;
	ssw[2] |= m_poll_required & 0x1;
	ssw[2] |= (m_reserved & 0x7F) << 1;
    }
    else
    {
	ssw[0] |= m_sectors & 0x3F;
	ssw[0] |= (m_antennas & 0x3) << 6;
	ssw[1] |= m_snr_report;
	ssw[2] |= m_poll_required & 0x1;
	ssw[2] |= (m_reserved & 0x7F) << 1;
    }

    i.Write(ssw, 3);
}

uint32_t DMG_SSW_FBCK_Field::Deserialize(Buffer::Iterator start)
{
    NS_LOG_FUNCTION(this << &start);
    Buffer::Iterator i = start;
    uint8_t ssw[3];
    i.Read(ssw, 3);

    if (m_is_iss)
    {
	m_sectors = (ssw[0] & 0xFF) | ((ssw[1] & 0x1) << 8);
	m_antennas = (ssw[1] >> 1) & 0x3;
	m_snr_report = (ssw[1] >> 3) & 0x1F;
	m_poll_required = ssw[2] & 0x1;
	m_reserved = (ssw[2] >> 1) & 0x7F;
    }
    else
    {
	m_sectors = ssw[0] & 0x3F;
	m_antennas = (ssw[0] >> 6) & 0x3;
	m_snr_report = ssw[1];
	m_poll_required = ssw[2] & 0x1;
	m_reserved = (ssw[2] >> 1) & 0x7F;
    }

    return i.GetDistanceFrom(start);
}

void DMG_SSW_FBCK_Field::SetSector(uint16_t value)
{
    NS_LOG_FUNCTION(this << value);
    m_sectors = value;
}

void DMG_SSW_FBCK_Field::SetDMGAntenna(uint8_t value)
{
    NS_LOG_FUNCTION(this << value);
    m_antennas = value;
}

void DMG_SSW_FBCK_Field::SetSNRReport(uint8_t value)
{
    NS_LOG_FUNCTION(this << value);
    m_snr_report = value;
}

void DMG_SSW_FBCK_Field::SetPollRequired(bool value)
{
    NS_LOG_FUNCTION(this << value);
    m_poll_required = value;
}

void DMG_SSW_FBCK_Field::SetReserved(uint8_t value)
{
    NS_LOG_FUNCTION(this << value);
    m_reserved = value;
}

void DMG_SSW_FBCK_Field::IsPartOfISS(bool value)
{
    NS_LOG_FUNCTION(this << value);
    m_is_iss = value;
}

uint16_t DMG_SSW_FBCK_Field::GetTotalSectorsInISS(void) const
{
    NS_LOG_FUNCTION(this);
    return m_sectors;
}

uint8_t DMG_SSW_FBCK_Field::GetNumberOfRXAntennas(void) const
{
    NS_LOG_FUNCTION(this);
    return m_antennas;
}

bool DMG_SSW_FBCK_Field::GetPollRequired(void) const
{
    NS_LOG_FUNCTION(this);
    return m_poll_required;
}

/***********************************
 *    BRP Request Field (8.4a.4).
 ***********************************/

NS_OBJECT_ENSURE_REGISTERED(BRP_Request_Field);

BRP_Request_Field::BRP_Request_Field() :
    m_L_RX(0),
    m_TX_TRN_REQ(false),
    m_MID_REQ(false),
    m_BC_REQ(false),
    m_MID_Grant(false),
    m_BC_Grant(false),
    m_Channel_FBCK_CAP(false),
    m_TXSectorID(0),
    m_OtherAID(0),
    m_TXAntennaID(0),
    m_Reserved(0)
{
    NS_LOG_FUNCTION(this);
}

BRP_Request_Field::~BRP_Request_Field()
{
    NS_LOG_FUNCTION(this);
}

TypeId BRP_Request_Field::GetTypeId(void)
{
    NS_LOG_FUNCTION_NOARGS();
    static TypeId tid = TypeId("ns3::BRP_Request_Field")
      .SetParent<Header>()
      .AddConstructor<BRP_Request_Field>()
    ;
    return tid;
}

TypeId BRP_Request_Field::GetInstanceTypeId(void) const
{
    NS_LOG_FUNCTION(this);
    return GetTypeId();
}

void BRP_Request_Field::Print(std::ostream &os) const
{
    NS_LOG_FUNCTION (this << &os);
    os << "L_RX=" << m_L_RX
       << ", TX_TRN_REQ=" << m_TX_TRN_REQ
       << ", MID_REQ=" << m_MID_REQ
       << ", BC_REQ=" << m_BC_REQ
       << ", MID_Grant=" << m_MID_Grant
       << ", BC_Grant=" << m_BC_Grant
       << ", Channel_FBCK_CAP=" << m_Channel_FBCK_CAP
       << ", TXSectorID=" << m_TXSectorID
       << ", OtherAID=" << m_OtherAID
       << ", TXAntennaID=" << m_TXAntennaID
       << ", Reserved=" << m_Reserved;
}

uint32_t BRP_Request_Field::GetSerializedSize() const
{
    NS_LOG_FUNCTION(this);
    return 32;
}

void BRP_Request_Field::Serialize(Buffer::Iterator start) const
{
    NS_LOG_FUNCTION(this << &start);
    Buffer::Iterator i = start;
    uint32_t brp = 0;

    brp |= m_L_RX & 0x1F;
    brp |= ((m_TX_TRN_REQ & 0x1) << 5);
    brp |= ((m_MID_REQ & 0x1) << 6);
    brp |= ((m_BC_REQ & 0x1) << 7);
    brp |= ((m_MID_Grant & 0x1) << 8);
    brp |= ((m_BC_Grant & 0x1) << 9);
    brp |= ((m_Channel_FBCK_CAP & 0x1) << 10);
    brp |= ((m_TXSectorID & 0x3F) << 11);
    brp |= (m_OtherAID << 17);
    brp |= ((m_TXAntennaID & 0x3) << 25);
    brp |= ((m_Reserved & 0x1F) << 27);

    i.WriteHtolsbU32(brp);
}

uint32_t BRP_Request_Field::Deserialize(Buffer::Iterator start)
{
    NS_LOG_FUNCTION(this << &start);
    Buffer::Iterator i = start;
    uint32_t brp = i.ReadLsbtohU32();

    m_L_RX =  brp & 0x1F;
    m_TX_TRN_REQ = (brp >> 5) & 0x1;
    m_MID_REQ = (brp >> 6) & 0x1;
    m_BC_REQ = (brp >> 7) & 0x1;
    m_MID_Grant = (brp >> 8) & 0x1;
    m_BC_Grant = (brp >> 9) & 0x1;
    m_Channel_FBCK_CAP = (brp >> 10) & 0x1;
    m_TXSectorID = (brp >> 11) & 0x3F;
    m_OtherAID = (brp >> 17) & 0xFF;
    m_TXAntennaID = (brp >> 25) & 0x3;
    m_Reserved = (brp >> 27) & 0x1F;

    return i.GetDistanceFrom(start);
}

void BRP_Request_Field::SetL_RX(uint8_t value)
{
    NS_LOG_FUNCTION(this << &value);
}

//void CtrlBRPRequestField::SetTX_TRN_REQ(bool value)
//{

//}

//void CtrlBRPRequestField::SetMID_REQ(bool value)
//{

//}

//void CtrlBRPRequestField::SetBC_REQ(bool value)
//{

//}

//void CtrlBRPRequestField::SetMID_Grant(bool value)
//{

//}

//void CtrlBRPRequestField::SetBC_Grant(bool value)
//{

//}

//void CtrlBRPRequestField::SetChannel_FBCK_CAP(bool value)
//{

//}

//void CtrlBRPRequestField::SetTXSectorID(uint8_t value)
//{

//}

//void CtrlBRPRequestField::SetOtherID(uint8_t value)
//{

//}

//void CtrlBRPRequestField::SetTXAntennaID(uint8_t value)
//{

//}

//void CtrlBRPRequestField::SetReserved(uint8_t value)
//{

//}

//uint8_t CtrlBRPRequestField::GetL_RX(void)
//{

//}

//bool CtrlBRPRequestField::GetTX_TRN_REQ(void)
//{

//}

//bool CtrlBRPRequestField::GetMID_REQ(void)
//{

//}

//bool CtrlBRPRequestField::GetBC_REQ(void)
//{

//}

//bool CtrlBRPRequestField::GetMID_Grant(void)
//{

//}

//bool CtrlBRPRequestField::GetBC_Grant(void)
//{

//}

//bool CtrlBRPRequestField::GetChannel_FBCK_CAP(void)
//{

//}

//uint8_t CtrlBRPRequestField::GetTXSectorID(void)
//{

//}

//uint8_t CtrlBRPRequestField::GetOtherID(void)
//{

//}

//uint8_t CtrlBRPRequestField::GetTXAntennaID(void)
//{

//}

//uint8_t CtrlBRPRequestField::GetReserved(void)
//{

//}

/***************************************
 * Beamforming Control Field (8.4a.5)
 ***************************************/

NS_OBJECT_ENSURE_REGISTERED(BF_Control_Field);

BF_Control_Field::BF_Control_Field()
  : m_beamformTraining(false),
    m_isInitiatorTXSS(false),
    m_isResponderTXSS(false),
    m_sectors(0), m_antennas(0),
    m_rxssLength(0), m_rxssTXRate(0),
    m_reserved(0)
{
    NS_LOG_FUNCTION(this);
}

BF_Control_Field::~BF_Control_Field()
{
    NS_LOG_FUNCTION(this);
}

TypeId BF_Control_Field::GetTypeId(void)
{
    NS_LOG_FUNCTION_NOARGS();
    static TypeId tid = TypeId("ns3::BF_Control_Field")
	.SetParent<Header>()
	.AddConstructor<BF_Control_Field>()
	;
    return tid;
}

TypeId BF_Control_Field::GetInstanceTypeId(void) const
{
    NS_LOG_FUNCTION(this);
    return GetTypeId();
}

void BF_Control_Field::Print(std::ostream &os) const
{
    NS_LOG_FUNCTION(this << &os);

    os << "Beamforming Training=" << m_beamformTraining
       << ", IsInitiatorTXSS=" << m_isInitiatorTXSS
       << ", IsResponderTXSS=" << m_isResponderTXSS;

    if (m_isInitiatorTXSS && m_isResponderTXSS)
    {
	os << ", Total Number of Sectors=" << m_sectors
	   << ", Number of RX DMG Antennas=" << m_antennas;
    }
    else
    {
	os << ", RXSS Length=" << m_rxssLength
	   << ", RXSSTxRate=" << m_rxssTXRate;
    }

    os << ", Reserved=" << m_reserved;
}

uint32_t BF_Control_Field::GetSerializedSize() const
{
    NS_LOG_FUNCTION(this);
    return 2;
}

void BF_Control_Field::Serialize(Buffer::Iterator start) const
{
    NS_LOG_FUNCTION (this << &start);
    Buffer::Iterator i = start;
    uint16_t value = 0;

    /* Common Subfields */
    value |= m_beamformTraining & 0x1;
    value |= ((m_isInitiatorTXSS & 0x1) << 1);
    value |= ((m_isResponderTXSS & 0x1) << 2);

    if (m_isInitiatorTXSS && m_isResponderTXSS)
    {
	value |= ((m_sectors & 0x7F) << 3);
	value |= ((m_antennas & 0x3) << 10);
	value |= ((m_reserved & 0xF) << 12);
    }
    else
    {
	value |= ((m_rxssLength & 0x3F) << 3);
	value |= ((m_rxssTXRate & 0x1) << 9);
	value |= ((m_reserved & 0x3F) << 10);
    }

    i.WriteHtolsbU16(value);
}

uint32_t BF_Control_Field::Deserialize(Buffer::Iterator start)
{
    NS_LOG_FUNCTION(this << &start);
    Buffer::Iterator i = start;
    uint8_t value = i.ReadLsbtohU16();

    m_beamformTraining = value & 0x1;
    m_isInitiatorTXSS = ((value >> 1) & 0x1);
    m_isResponderTXSS = ((value >> 2) & 0x1);

    if (m_isInitiatorTXSS && m_isResponderTXSS)
    {
	m_sectors |= ((value >> 3) & 0x7F);
	m_antennas |= ((value >> 10) & 0x3);
	m_reserved |= ((value >> 12) & 0xF);
    }
    else
    {
	m_rxssLength |= ((value >> 3) & 0x3F);
	m_rxssTXRate |= ((value >> 9) & 0x1);
	m_reserved |= ((value >> 10) & 0x3F);
    }

    return i.GetDistanceFrom(start);
}

/***************************************
 * Beamformed Link Maintenance (8.4a.6)
 ***************************************/

NS_OBJECT_ENSURE_REGISTERED(BF_Link_Maintenance_Field);

BF_Link_Maintenance_Field::BF_Link_Maintenance_Field()
  : m_unitIndex(0),
    m_value(0),
    m_isMaster(false)
{
    NS_LOG_FUNCTION(this);
}

BF_Link_Maintenance_Field::~BF_Link_Maintenance_Field()
{
    NS_LOG_FUNCTION(this);
}

TypeId BF_Link_Maintenance_Field::GetTypeId(void)
{
    NS_LOG_FUNCTION_NOARGS();
    static TypeId tid = TypeId("ns3::BF_Link_Maintenance_Field")
	.SetParent<Header>()
	.AddConstructor<BF_Link_Maintenance_Field>()
	;
    return tid;
}

TypeId BF_Link_Maintenance_Field::GetInstanceTypeId(void) const
{
    NS_LOG_FUNCTION(this);
    return GetTypeId();
}

void BF_Link_Maintenance_Field::Print(std::ostream &os) const
{
    NS_LOG_FUNCTION(this << &os);
    os << "Unit Index=" << m_unitIndex
       << ", Value=" << m_value
       << ", isMaster=" << m_isMaster;
}

uint32_t BF_Link_Maintenance_Field::GetSerializedSize() const
{
    NS_LOG_FUNCTION(this);
    return 1;
}

void BF_Link_Maintenance_Field::Serialize(Buffer::Iterator start) const
{
    NS_LOG_FUNCTION (this << &start);
    Buffer::Iterator i = start;
    uint8_t value = 0;

    value |= m_unitIndex & 0x1;
    value |= ((m_value & 0x3F) << 1);
    value |= ((m_isMaster & 0x1) << 7);

    i.WriteU8(value);
}

uint32_t BF_Link_Maintenance_Field::Deserialize(Buffer::Iterator start)
{
    NS_LOG_FUNCTION(this << &start);
    Buffer::Iterator i = start;
    uint8_t value = i.ReadU8();

    m_unitIndex = value & 0x1;
    m_value = ((value >> 1) & 0x3F);
    m_isMaster = ((value >> 7) & 0x1);

    return i.GetDistanceFrom(start);
}

void BF_Link_Maintenance_Field::SetUnitIndex(bool index)
{
    NS_LOG_FUNCTION (this << index);
    m_unitIndex = index;
}

void BF_Link_Maintenance_Field::SetMaintenanceValue(uint8_t value)
{
    NS_LOG_FUNCTION(this << value);
    m_value = value;
}

void BF_Link_Maintenance_Field::SetIsMaster(bool value)
{
    NS_LOG_FUNCTION(this << value);
    m_isMaster = value;
}

bool BF_Link_Maintenance_Field::GetUnitIndex(void) const
{
    NS_LOG_FUNCTION(this);
    return m_unitIndex;
}

uint8_t BF_Link_Maintenance_Field::GetMaintenanceValue(void) const
{
    NS_LOG_FUNCTION(this);
    return m_value;
}

bool BF_Link_Maintenance_Field::GetIsMaster(void) const
{
    NS_LOG_FUNCTION(this);
    return m_isMaster;
}

}  // namespace ns3
