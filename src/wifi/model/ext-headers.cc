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
 * Author: Hany Assasa <hany.assasa@hotmail.com
 */
#include "ns3/fatal-error.h"
#include "ns3/log.h"
#include "ns3/simulator.h"

#include "ext-headers.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("ExtHeaders");

/******************************************
*	DMG Parameters Field (8.4.1.46)
*******************************************/

NS_OBJECT_ENSURE_REGISTERED (ExtDMGParameters);

ExtDMGParameters::ExtDMGParameters()
{}

ExtDMGParameters::~ExtDMGParameters()
{}

TypeId ExtDMGParameters::GetTypeId (void)
{
    static TypeId tid = TypeId ("ns3::ExtDMGParameters")
	.SetParent<Header>()
	.AddConstructor<ExtDMGParameters>()
	;
    return tid;
}

TypeId ExtDMGParameters::GetInstanceTypeId(void) const
{
    return GetTypeId();
}

uint32_t ExtDMGParameters::GetSerializedSize(void) const
{
    return 1; // 1 bytes length.
}

void ExtDMGParameters::Print(std::ostream &os) const
{

}

void ExtDMGParameters::Serialize(Buffer::Iterator start) const
{
    NS_LOG_FUNCTION(this << &start);
    Buffer::Iterator i = start;
    uint8_t buffer = 0;

    buffer |= m_bssType & 0x3;
    buffer |= ((m_cbapOnly & 0x1) << 2);
    buffer |= ((m_cbapSource & 0x1) << 3);
    buffer |= ((m_dmgPrivacy & 0x1) << 4);
    buffer |= ((m_ecpacPolicyEnforced & 0x1) << 5);
    buffer |= ((m_reserved & 0x3) << 6);

    i.WriteU8(buffer);
}

uint32_t ExtDMGParameters::Deserialize(Buffer::Iterator start)
{
    NS_LOG_FUNCTION(this << &start);
    Buffer::Iterator i = start;
    uint8_t buffer = i.ReadU8();

    m_bssType = static_cast<enum BSSType> (buffer & 0x3);
    m_cbapOnly = (buffer >> 2) & 0x1;
    m_cbapSource = (buffer >> 3) & 0x1;
    m_dmgPrivacy = (buffer >> 4) & 0x1;
    m_ecpacPolicyEnforced = (buffer >> 5) & 0x1;
    m_reserved = (buffer >> 6) & 0x3;

    return i.GetDistanceFrom(start);
}

/**
* Set the duration of the beacon frame.
*
* \param duration The duration of the beacon frame
*/
void ExtDMGParameters::Set_BSS_Type(enum BSSType type)
{
    NS_LOG_FUNCTION(this << type);
    m_bssType = type;
}

void ExtDMGParameters::Set_CBAP_Only(bool value)
{
    NS_LOG_FUNCTION(this << value);
    m_cbapOnly = value;
}

void ExtDMGParameters::Set_CBAP_Source(bool value)
{
    NS_LOG_FUNCTION(this << value);
    m_cbapSource = value;
}

void ExtDMGParameters::Set_DMG_Privacy(bool value)
{
    NS_LOG_FUNCTION(this << value);
    m_dmgPrivacy = value;
}

void ExtDMGParameters::Set_ECPAC_Policy_Enforced(bool value)
{
    NS_LOG_FUNCTION(this << value);
    m_ecpacPolicyEnforced = value;
}

void ExtDMGParameters::Set_Reserved(uint8_t value)
{
    NS_LOG_FUNCTION(this << value);
    m_reserved = value;
}

/**
* Return the Basic Service Set (BSS) Type.
*
* \return BSSType
*/
enum BSSType ExtDMGParameters::Get_BSS_Type(void) const
{
    NS_LOG_FUNCTION(this);
    return m_bssType;
}

bool ExtDMGParameters::Get_CBAP_Only(void) const
{
    NS_LOG_FUNCTION(this);
    return m_cbapOnly;
}

bool ExtDMGParameters::Get_CBAP_Source(void) const
{
    NS_LOG_FUNCTION(this);
    return m_cbapSource;
}

bool ExtDMGParameters::Get_DMG_Privacy(void) const
{
    NS_LOG_FUNCTION(this);
    return m_dmgPrivacy;
}

bool ExtDMGParameters::Get_ECPAC_Policy_Enforced(void) const
{
    NS_LOG_FUNCTION(this);
    return m_ecpacPolicyEnforced;
}

uint8_t ExtDMGParameters::Get_Reserved(void) const
{
    NS_LOG_FUNCTION(this);
    return m_reserved;
}

/******************************************
*   Beacon Interval Control Field (8-34b)
*******************************************/

NS_OBJECT_ENSURE_REGISTERED (ExtDMGBeaconIntervalCtrlField);

ExtDMGBeaconIntervalCtrlField::ExtDMGBeaconIntervalCtrlField()
{}

ExtDMGBeaconIntervalCtrlField::~ExtDMGBeaconIntervalCtrlField()
{}

TypeId ExtDMGBeaconIntervalCtrlField::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::ExtDMGBeaconIntervalCtrlField")
    .SetParent<Header>()
    .AddConstructor<ExtDMGBeaconIntervalCtrlField>()
  ;
  return tid;
}

TypeId ExtDMGBeaconIntervalCtrlField::GetInstanceTypeId(void) const
{
    return GetTypeId();
}

uint32_t ExtDMGBeaconIntervalCtrlField::GetSerializedSize(void) const
{
    return 6; // 6 bytes length.
}

void ExtDMGBeaconIntervalCtrlField::Print(std::ostream &os) const
{

}

void ExtDMGBeaconIntervalCtrlField::Serialize(Buffer::Iterator start) const
{
    //NS_LOG_FUNCTION(this << &start);
    Buffer::Iterator i = start;
    uint32_t ctrl1 = 0;
    uint16_t ctrl2 = 0;

    ctrl1 |= m_ccPresent & 0x1;
    ctrl1 |= ((m_discoveryMode & 0x1) << 1);
    ctrl1 |= ((m_nextBeacon & 0xF) << 2);
    ctrl1 |= ((m_ATI_Present & 0x1) << 6);
    ctrl1 |= ((m_ABFT_Length & 0x7) << 7);
    ctrl1 |= ((m_FSS & 0xF) << 10);
    ctrl1 |= ((m_isResponderTXSS & 0x1) << 14);
    ctrl1 |= ((m_next_ABFT & 0xF) << 15);
    ctrl1 |= ((m_fragmentedTXSS & 0x1) << 19);
    ctrl1 |= ((m_TXSS_Span & 0x3F) << 20);
    ctrl1 |= ((m_N_BI & 0xF) << 27);
    ctrl1 |= ((m_ABFT_Count & 0x1) << 31);

    ctrl2 |= (m_ABFT_Count >> 1) & 0x1F;
    ctrl2 |= ((m_N_ABFT_Ant & 0x3F) << 5);
    ctrl2 |= ((m_pcpAssociationReady & 0x1) << 11);
    ctrl2 |= ((m_reserved & 0xF) << 12);

    i.WriteHtolsbU32(ctrl1);
    i.WriteHtolsbU16(ctrl2);
}

uint32_t ExtDMGBeaconIntervalCtrlField::Deserialize(Buffer::Iterator start)
{
    NS_LOG_FUNCTION(this << &start);
    Buffer::Iterator i = start;
    uint32_t ctrl1 = i.ReadLsbtohU32();
    uint16_t ctrl2 = i.ReadLsbtohU16();

    m_ccPresent = ctrl1 & 0x1;
    m_discoveryMode = (ctrl1 >> 1) & 0x1;
    m_nextBeacon = (ctrl1 >> 2) & 0xF;
    m_ATI_Present = (ctrl1 >> 6) & 0x1;
    m_ABFT_Length = (ctrl1 >> 7) & 0x1;
    m_FSS = (ctrl1 >> 10) & 0x7;
    m_isResponderTXSS = (ctrl1 >> 14) & 0x1;
    m_next_ABFT = (ctrl1 >> 15) & 0xF;
    m_fragmentedTXSS = (ctrl1 >> 19) & 0x1;
    m_TXSS_Span = (ctrl1 >> 20) & 0x3F;
    m_N_BI = (ctrl1 >> 27) & 0xF;
    m_ABFT_Count =  ((ctrl1 >> 31) & 0x1) | ((ctrl2 << 1) & 0x3E);
    m_N_ABFT_Ant = (ctrl2 >> 5) & 0x3F;
    m_pcpAssociationReady = (ctrl2 >> 11) & 0x1;
    m_reserved = (ctrl2 >> 12) & 0xF;

    return i.GetDistanceFrom(start);
}


/******************************************
*	    DMG Beacon (8.3.4.1)
*******************************************/

NS_OBJECT_ENSURE_REGISTERED(ExtDMGBeacon);

ExtDMGBeacon::ExtDMGBeacon()
{}

ExtDMGBeacon::~ExtDMGBeacon()
{}

uint64_t ExtDMGBeacon::GetTimestamp() const
{
    return m_timestamp;
}

TypeId ExtDMGBeacon::GetTypeId (void)
{
    static TypeId tid = TypeId ("ns3::ExtDMGBeacon")
	.SetParent<Header>()
	.AddConstructor<ExtDMGBeacon>()
	;
    return tid;
}

TypeId ExtDMGBeacon::GetInstanceTypeId(void) const
{
    return GetTypeId();
}

uint32_t ExtDMGBeacon::GetSerializedSize(void) const
{
    uint32_t size = 0;

    size += 8; // Timestamp (See 8.4.1.10)
    size += 3; // Sector Sweep (See 8.4a.1)
    size += 2; // Beacon Interval (See 8.4.1.3)
    size += 6; // Beacon Interval Control (See 8.4.1.3)
    size += 1; // DMG Parameters (See 8.4.1.46)

    return size;
}

void ExtDMGBeacon::Print(std::ostream &os) const
{

}

void ExtDMGBeacon::Serialize(Buffer::Iterator start) const
{
    // Timestamp.
    // Sector Sweep.
    // Beacon Interval.
    // Beacon Interval Control.
    // DMG Parameters.
    Buffer::Iterator i = start;

    i.WriteHtolsbU64(Simulator::Now().GetMicroSeconds());
    m_ssw.Serialize(i);
    i.WriteHtolsbU16(m_beaconInterval / 1024);
    m_beaconIntervalCtrl.Serialize(i);
    m_dmgParameters.Serialize(i);
}

uint32_t ExtDMGBeacon::Deserialize(Buffer::Iterator start)
{
    Buffer::Iterator i = start;

    m_timestamp = i.ReadLsbtohU64();
    m_ssw.Deserialize(i);
    m_beaconInterval = i.ReadLsbtohU16();
    m_beaconInterval *= 1024;
    m_beaconIntervalCtrl.Deserialize(i);
    m_dmgParameters.Deserialize(i);

    return i.GetDistanceFrom(start);
}


} // namespace ns3
