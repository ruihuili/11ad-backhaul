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
 * Author: Hany Assasa <hany.assasa@gmail.com>
 */
#ifndef EXT_HEADERS_H
#define EXT_HEADERS_H

#include "ns3/header.h"
#include "ns3/mac48-address.h"

#include "fields-headers.h"

namespace ns3 {

enum BSSType
{
    Reserved = 0,
    IBSS = 1,
    PBSS = 2,
    InfrastructureBSS = 3
};

/******************************************
*   Beacon Interval Control Field (8-34b)
*******************************************/

/**
 * \ingroup wifi
 * Implement the header for DMG Beacon.
 */
class ExtDMGBeaconIntervalCtrlField : public Header
{
public:
    ExtDMGBeaconIntervalCtrlField();
    ~ExtDMGBeaconIntervalCtrlField();
    static TypeId GetTypeId(void);
    virtual TypeId GetInstanceTypeId(void) const;
    virtual void Print(std::ostream &os) const;
    virtual uint32_t GetSerializedSize(void) const;
    virtual void Serialize(Buffer::Iterator start) const;
    virtual uint32_t Deserialize(Buffer::Iterator start);

    /**
     * Set the duration of the beacon frame.
     *
     * \param duration The duration of the beacon frame
     */
    void SetCCPresent(bool value);
    void SetDiscoveryMode(bool value);
    void SetNextBeacon(uint8_t value);
    void SetATIPresent(bool value);
    void SetABFT_Length(uint8_t length);
    void SetFSS(uint8_t value);
    void SetIsResponderTXSS(bool value);
    void SetNextABFT(uint8_t value);
    void SetFragmentedTXSS(bool value);
    void SetTXSS_Span(uint8_t value);
    void SetN_BI(uint8_t value);
    void SetABFT_Count(uint8_t value);
    void SetN_ABFT_Ant(uint8_t value);
    void SetPCPAssoicationReady(bool value);
    void SetReserved(uint8_t value);

    /**
    * Return the Service Set Identifier (SSID).
    *
    * \return SSID
    */
    bool GetCCPresent(void);
    bool GetDiscoveryMode(void);
    uint8_t GetNextBeacon(void);
    bool GetATIPresent(void);
    uint8_t GetABFT_Length(void);
    uint8_t GetFSS(void);
    bool GetIsResponderTXSS(void);
    uint8_t GetNextABFT(void);
    bool GetFragmentedTXSS(void);
    uint8_t GetTXSS_Span(void);
    uint8_t GetN_BI(void);
    uint8_t GetABFT_Count(void);
    uint8_t GetN_ABFT_Ant(void);
    bool GetPCPAssoicationReady(void);
    uint8_t GetReserved(void);

private:
    bool m_ccPresent;
    bool m_discoveryMode;
    uint8_t m_nextBeacon;
    bool m_ATI_Present;
    uint8_t m_ABFT_Length;
    uint8_t m_FSS;
    bool m_isResponderTXSS;
    uint8_t m_next_ABFT;
    bool m_fragmentedTXSS;
    uint8_t m_TXSS_Span;
    uint8_t m_N_BI;
    uint8_t m_ABFT_Count;
    uint8_t m_N_ABFT_Ant;
    bool m_pcpAssociationReady;
    uint8_t m_reserved;

};

/******************************************
*	DMG Parameters Field (8.4.1.46)
*******************************************/

/**
 * \ingroup wifi
 * Implement the header for DMG Parameters Field.
 */
class ExtDMGParameters : public Header
{
public:
    ExtDMGParameters();
    ~ExtDMGParameters();
    static TypeId GetTypeId(void);
    virtual TypeId GetInstanceTypeId(void) const;
    virtual void Print(std::ostream &os) const;
    virtual uint32_t GetSerializedSize(void) const;
    virtual void Serialize(Buffer::Iterator start) const;
    virtual uint32_t Deserialize(Buffer::Iterator start);

    /**
     * Set the Basic Service Set (BSS) Type.
     *
     * \param duration The duration of the beacon frame
     */
    void Set_BSS_Type(enum BSSType type);
    void Set_CBAP_Only(bool value);
    void Set_CBAP_Source(bool value);
    void Set_DMG_Privacy(bool value);
    void Set_ECPAC_Policy_Enforced(bool length);
    void Set_Reserved(uint8_t value);

    /**
    * Return the Basic Service Set (BSS) Type.
    *
    * \return BSSType
    */
    enum BSSType Get_BSS_Type(void) const;
    bool Get_CBAP_Only(void) const;
    bool Get_CBAP_Source(void) const;
    bool Get_DMG_Privacy(void) const;
    bool Get_ECPAC_Policy_Enforced(void) const;
    uint8_t Get_Reserved(void) const;

private:
    enum BSSType m_bssType;	    //!< BSS Type.
    bool m_cbapOnly;		    //!< CBAP Only.
    bool m_cbapSource;		    //!< CBAP Source.
    bool m_dmgPrivacy;		    //!< DMG Privacy.
    bool m_ecpacPolicyEnforced;	    //!< ECPAC Policy Enforced.
    uint8_t m_reserved;

};

/******************************************
*	     DMG Beacon (8.3.4.1)
*******************************************/

/**
 * \ingroup wifi
 * Implement the header for DMG Beacon.
 */
class ExtDMGBeacon : public Header
{
public:
    ExtDMGBeacon();
    ~ExtDMGBeacon();

    /**
     * Set the duration of the beacon frame.
     *
     * \param duration The duration of the beacon frame
     */
    void SetDuration(uint16_t duration);
    /**
    * Set the Basic Service Set Identifier (BSSID).
    *
    * \param ssid BSSID
    */
    void SetBSSID(Mac48Address bssid);
    /**
    * Set the Timestamp in the DMG Beacon frame body.
    *
    * \param timestamp The timestamp.
    */
    void SetTimestamp(uint64_t timestamp);
    /**
    * Set Sector Sweep Information Field in the DMG Beacon frame body.
    *
    * \param ssw The sector sweep information field.
    */
    void SetSSWField(DMG_SSW_Field ssw);
    /**
    * Set the DMG Beacon Interval.
    *
    * \param interval The DMG Beacon Interval.
    */
    void SetBeaconInterval(uint16_t interval);
    /**
    * Set Beacon Interval Control Field in the DMG Beacon frame body.
    *
    * \param ssw The Beacon Interval Control field.
    */
    void SetBeaconIntervalControlField(ExtDMGBeaconIntervalCtrlField ctrl);
    /**
    * Set DMG Parameters Field in the DMG Beacon frame body.
    *
    * \param parameters The DMG Parameters field.
    */
    void SetBeaconIntervalControlField(ExtDMGParameters parameters);

    /**
    * Return the Basic Service Set Identifier (BSSID).
    *
    * \return Duration
    */
    uint16_t GetDuration(void) const;
    /**
    * Return the Service Set Identifier (SSID).
    *
    * \return SSID
    */
    Mac48Address GetBSSID(void) const;
    /**
    * Get the Timestamp in the DMG Beacon frame body.
    *
    * \return The timestamp.
    */
    uint64_t GetTimestamp(void) const;
    /**
    * Get Sector Sweep Information Field in the DMG Beacon frame body.
    *
    * \return The sector sweep information field.
    */
    DMG_SSW_Field GetSSWField(void);
    /**
    * Get the DMG Beacon Interval.
    *
    * \return The DMG Beacon Interval.
    */
    uint16_t GetBeaconInterval(void) const;
    /**
    * Get Beacon Interval Control Field in the DMG Beacon frame body.
    *
    * \return The Beacon Interval Control field.
    */
    ExtDMGBeaconIntervalCtrlField GetBeaconIntervalControlField(void);
    /**
    * Get DMG Parameters Field in the DMG Beacon frame body.
    *
    * \return The DMG Parameters field.
    */
    ExtDMGParameters GetDMGParameters(void);

    static TypeId GetTypeId(void);
    virtual TypeId GetInstanceTypeId(void) const;
    virtual void Print(std::ostream &os) const;
    virtual uint32_t GetSerializedSize(void) const;
    virtual void Serialize(Buffer::Iterator start) const;
    virtual uint32_t Deserialize(Buffer::Iterator start);

private:
    Mac48Address m_bssid;	    //!< Basic Service Set ID (SSID).
    uint64_t m_timestamp;	    //!< Timestamp.
    DMG_SSW_Field m_ssw;	    //!< Sector Sweep Field.
    uint16_t m_beaconInterval;	    //!< Beacon Interval.
    ExtDMGBeaconIntervalCtrlField m_beaconIntervalCtrl; //!< Beacon Interval Control.
    ExtDMGParameters m_dmgParameters; //!< DMG Parameters.

};

} // namespace ns3

#endif /* EXT_HEADERS_H */
