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
#ifndef FIELDS_HEADERS_H
#define FIELDS_HEADERS_H

#include "ns3/header.h"

namespace ns3 {

/***********************************
 *    Sector Sweep Field (8.4a.1)
 ***********************************/

enum Direction
{
    BeamformingInitiator = 0,
    BeamformingResponder = 1
};

/**
 * \ingroup wifi
 * \brief Sector Sweep (SSW) Field.
 */
class DMG_SSW_Field : public Header
{
public:
    DMG_SSW_Field();
    ~DMG_SSW_Field();
    static TypeId GetTypeId(void);
    virtual TypeId GetInstanceTypeId(void) const;
    virtual void Print(std::ostream &os) const;
    virtual uint32_t GetSerializedSize(void) const;
    virtual void Serialize(Buffer::Iterator start) const;
    virtual uint32_t Deserialize(Buffer::Iterator start);

    /**
    * Set the direction field in the SSW field.
    *
    * \param dir 0 = the frame is transmiited by the beamforming initiator.
    *		 1 = the frame is transmiited by the beamforming responder.
    */
    void SetDirection(enum Direction dir);
    /**
    * Set the Count Down (CDOWN) field.
    *
    * \param cdown the number of remaining DMG beacon frame transmissions to the end of TXSS,
    * or the number of the remaining SSW frame transmissions to the end of the TXSS/RXSS.
    */
    void SetCountDown(uint16_t cdown);
    /**
    * Set Sector ID (SID).
    *
    * \param sid indicates the sector number through which the frame containing this SSW field is transmitted.
    */
    void SetSectorID(uint8_t sid);
    /**
    * Set the DMG Antenna ID.
    *
    * \param antenna_id the DMG antenna the transmitter is currently using for this transmission.
    */
    void SetDMGAntennaID(uint8_t antenna_id);
    /**
    * Set the Receive Sector Sweep (RXSS) Length.
    *
    * \param length the length of a receive sector sweep as required by the transmitting STA.
    * It is defined in units of a SSW frame
    */
    void SetRXSSLength(uint8_t length);

    /**
    * Get the direction field in the SSW field.
    *
    * \return Whether the frame is transmitted by Beamforming Initiator or Responder.
    */
    enum Direction GetDirection(void) const;
    /**
    * Get the Count Down (CDOWN) field value.
    *
    * \return the number of remaining DMG beacon frame transmissions to the end of TXSS,
    * or the number of the remaining SSW frame transmissions to the end of the TXSS/RXSS.
    */
    uint16_t GetCountDown(void) const;
    /**
    * Get Sector ID (SID) value.
    *
    * \return the sector number through which the frame containing this SSW field is transmitted.
    */
    uint8_t GetSectorID(void) const;
    /**
    * Get the DMG Antenna ID value.
    *
    * \return the DMG antenna the transmitter is currently using for this transmission.
    */
    uint8_t GetDMGAntennaID(void) const;
    /**
    * Get the RXSS Length.
    *
    * \return the length of a receive sector sweep as required by the transmitting STA.
    * It is defined in units of a SSW frame
    */
    uint8_t GetRXSSLength(void) const;

private:
    enum Direction m_dir;
    uint16_t m_cdown;
    uint8_t m_sid;
    uint8_t m_antenna_id;
    uint8_t m_length;

};

/****************************************
 * Dynamic Allocation Info Field (8.4a.2)
 ****************************************/

enum AllocationType
{
    SP_Allocation = 0,
    CBAP_Allocation = 1
};

/**
 * \ingroup wifi
 * \brief Dynamic Allocation Info Field.
 */
class Dynamic_Allocation_Info_Field : public Header
{
public:
    Dynamic_Allocation_Info_Field();
    ~Dynamic_Allocation_Info_Field();
    static TypeId GetTypeId(void);
    virtual TypeId GetInstanceTypeId(void) const;
    virtual void Print(std::ostream &os) const;
    virtual uint32_t GetSerializedSize(void) const;
    virtual void Serialize(Buffer::Iterator start) const;
    virtual uint32_t Deserialize(Buffer::Iterator start);

    /**
    * Set the TID field that identifies the TC or TS for the allocation request or grant.
    *
    * \param tid The value of the Traffic Identifier field.
    */
    void SetTID(uint8_t tid);
    /**
    * Set the allocation type field.
    *
    * \param
    */
    void SetAllocationType(enum AllocationType value);
    /**
    * Set Source AID.
    *
    * \param
    */
    void SetSourceAID(uint8_t aid);
    /**
    * Set the Destination AID.
    *
    * \param
    */
    void SetDestinationAID(uint8_t aid);
    /**
    * Set the Allocation Duration.
    *
    * \param
    */
    void SetAllocationDuration(uint16_t duration);
    /**
    * Set the Reserved Field value.
    *
    * \param
    */
    void SetReserved(uint8_t length);

    /**
    *
    *
    * \return
    */
    uint8_t GetTID(void) const;
    /**
    *
    *
    * \return
    */
    enum AllocationType GetAllocationType(void) const;
    /**
    *
    *
    * \return
    */
    uint8_t GetSourceAID(void) const;
    /**
    *
    *
    * \return
    */
    uint8_t GetDestinationAID(void) const;
    /**
    *
    *
    * \return
    */
    uint16_t GetAllocationDuration(void) const;
    /**
    * Get the Reserved Field value.
    *
    * \param
    */
    uint8_t GetReserved(void);

private:
    uint8_t m_tid;
    enum AllocationType m_allocationType;
    uint8_t m_source_AID;
    uint8_t m_destination_AID;
    uint8_t m_allocation_duration;
    uint8_t m_reserved;

};

/*******************************************
 *    Sector Sweep Feedback Field (8.4a.3)
 *******************************************/

/**
 * \ingroup wifi
 * \brief Sector Sweep Feedback Field.
 */
class DMG_SSW_FBCK_Field : public Header
{
public:
    DMG_SSW_FBCK_Field();
    ~DMG_SSW_FBCK_Field();
    static TypeId GetTypeId(void);
    virtual TypeId GetInstanceTypeId(void) const;
    virtual void Print(std::ostream &os) const;
    virtual uint32_t GetSerializedSize(void) const;
    virtual void Serialize(Buffer::Iterator start) const;
    virtual uint32_t Deserialize(Buffer::Iterator start);

    /**
    * Set the total number of sectors the initiator uses in the ISS or the ID of
    * the frame that was received with best quality in the preceding sector sweep.
    *
    * \param value the total number of sectors the initiator uses in the ISS or the ID of the best sector.
    */
    void SetSector(uint16_t value);
    /**
    * Set the number of RX DMG Antennas in ISS or Select DMG Antenna in case
    *
    * \param antennas the number of RX DMG antennas the initiator uses during the following RSS.
    */
    void SetDMGAntenna(uint8_t antennas);
    /**
    * Set the SNR Report in case not ISS or the reserved value in case ISS.
    *
    * \param antennas
    */
    void SetSNRReport(uint8_t antennas);
    /**
    * Set whether a non-PCP/non-AP STA requires the PCP/AP to initiate the communication.
    *
    * \param value
    */
    void SetPollRequired(bool value);
    /**
    * Set the value of the reserved subfield in the SSW Feedback field.
    *
    * \param value
    */
    void SetReserved(uint8_t value);
    /**
    * Set whether the SSW Feedback Field is transmitted as part of ISS.
    *
    * \param value True if the SSW Feedback Field is transmitted as part of ISS, otherwise false.
    */
    void IsPartOfISS(bool value);

    /**
    * Get the direction field in the SSW field.
    *
    * \return Whether the frame is transmitted by Beamforming Initiator or Responder.
    */
    uint16_t GetTotalSectorsInISS(void) const;
    /**
    * Get the Count Down (CDOWN) field value.
    *
    * \return the number of remaining DMG beacon frame transmissions to the end of TXSS,
    * or the number of the remaining SSW frame transmissions to the end of the TXSS/RXSS.
    */
    uint8_t GetNumberOfRXAntennas(void) const;
    /**
    * Get Sector ID (SID) value.
    *
    * \return the sector number through which the frame containing this SSW field is transmitted.
    */
    bool GetPollRequired(void) const;

private:
    uint16_t m_sectors;
    uint8_t m_antennas;
    uint8_t m_snr_report;
    bool m_poll_required;
    uint8_t m_reserved;
    bool m_is_iss;

};

/***********************************
 *    BRP Request Field (8.4a.4).
 ***********************************/

/**
 * \ingroup wifi
 * \brief Beam Refinement Protocol Request Field.
 */
class BRP_Request_Field : public Header
{
public:
    BRP_Request_Field();
    ~BRP_Request_Field();
    static TypeId GetTypeId(void);
    virtual TypeId GetInstanceTypeId(void) const;
    virtual void Print(std::ostream &os) const;
    virtual uint32_t GetSerializedSize(void) const;
    virtual void Serialize(Buffer::Iterator start) const;
    virtual uint32_t Deserialize(Buffer::Iterator start);

    void SetL_RX(uint8_t value);
    void SetTX_TRN_REQ(bool value);
    void SetMID_REQ(bool value);
    void SetBC_REQ(bool value);
    void SetMID_Grant(bool value);
    void SetBC_Grant(bool value);
    void SetChannel_FBCK_CAP(bool value);
    void SetTXSectorID(uint8_t value);
    void SetOtherAID(uint8_t value);
    void SetTXAntennaID(uint8_t value);
    void SetReserved(uint8_t value);

    uint8_t GetL_RX(void);
    bool GetTX_TRN_REQ(void);
    bool GetMID_REQ(void);
    bool GetBC_REQ(void);
    bool GetMID_Grant(void);
    bool GetBC_Grant(void);
    bool GetChannel_FBCK_CAP(void);
    uint8_t GetTXSectorID(void);
    uint8_t GetOtherAID(void);
    uint8_t GetTXAntennaID(void);
    uint8_t GetReserved(void);

private:
    uint8_t m_L_RX;
    bool m_TX_TRN_REQ;
    bool m_MID_REQ;
    bool m_BC_REQ;
    bool m_MID_Grant;
    bool m_BC_Grant;
    bool m_Channel_FBCK_CAP;
    uint8_t m_TXSectorID;
    uint8_t m_OtherAID;
    uint8_t m_TXAntennaID;
    uint8_t m_Reserved;

};

/***************************************
 * Beamforming Control Field (8.4a.5)
 ***************************************/

/**
 * \ingroup wifi
 * \brief The Beamforming Control Field.
 */
class BF_Control_Field : public Header
{
public:
    BF_Control_Field();
    ~BF_Control_Field();
    static TypeId GetTypeId(void);
    virtual TypeId GetInstanceTypeId(void) const;
    virtual void Print(std::ostream &os) const;
    virtual uint32_t GetSerializedSize(void) const;
    virtual void Serialize(Buffer::Iterator start) const;
    virtual uint32_t Deserialize(Buffer::Iterator start);

    /**
    * Set the encoding of the BeamLink Maintenance Unit Index.
    *
    * \param index The encoding of the BeamLink Maintenance Unit Index.
    */
    void SetUnitIndex(bool index);
    /**
    * Set the number of RX DMG Antennas in ISS or Select DMG Antenna in case
    *
    * \param value The number of RX DMG antennas the initiator uses during the following RSS.
    */
    void SetMaintenanceValue(uint8_t value);
    /**
    * Set the BeamLink isMaster field is set to 1 to indicate that the DMG STA is the
    * master of the data transfer and set to 0 if the DMG STA is a slave of the data transfer.
    *
    * \param value Is DMG STA is the master or not.
    */
    void SetIsMaster(bool value);

    /**
    *
    *
    * \return
    */
    bool GetUnitIndex(void) const;
    /**
    *
    *
    * \return
    */
    uint8_t GetMaintenanceValue(void) const;
    /**
    *
    *
    * \return
    */
    bool GetIsMaster(void) const;

private:
    bool m_beamformTraining;
    bool m_isInitiatorTXSS;
    bool m_isResponderTXSS;

    /* BF Control Fields when both IsInitiatorTXSS and IsResponderTXSS subfields are
     * equal to 1 and the BF Control field is transmitted in Grant or Grant ACK frames */
    uint8_t m_sectors;
    uint8_t m_antennas;

    /* BF Control field format in all other cases */
    uint8_t m_rxssLength;
    bool m_rxssTXRate;

    uint8_t m_reserved;

};

/***************************************
 * Beamformed Link Maintenance (8.4a.6)
 ***************************************/

/**
 * \ingroup wifi
 * \brief The Beamformed Link Maintenance provides the DMG STA with the value of dot11BeamLinkMaintenanceTime.
 */
class BF_Link_Maintenance_Field : public Header
{
public:
    BF_Link_Maintenance_Field();
    ~BF_Link_Maintenance_Field();
    static TypeId GetTypeId(void);
    virtual TypeId GetInstanceTypeId(void) const;
    virtual void Print(std::ostream &os) const;
    virtual uint32_t GetSerializedSize(void) const;
    virtual void Serialize(Buffer::Iterator start) const;
    virtual uint32_t Deserialize(Buffer::Iterator start);

    /**
    * Set the encoding of the BeamLink Maintenance Unit Index.
    *
    * \param index The encoding of the BeamLink Maintenance Unit Index.
    */
    void SetUnitIndex(bool index);
    /**
    * Set the number of RX DMG Antennas in ISS or Select DMG Antenna in case
    *
    * \param value The number of RX DMG antennas the initiator uses during the following RSS.
    */
    void SetMaintenanceValue(uint8_t value);
    /**
    * Set the BeamLink isMaster field is set to 1 to indicate that the DMG STA is the
    * master of the data transfer and set to 0 if the DMG STA is a slave of the data transfer.
    *
    * \param value Is DMG STA is the master or not.
    */
    void SetIsMaster(bool value);

    /**
    *
    *
    * \return
    */
    bool GetUnitIndex(void) const;
    /**
    *
    *
    * \return
    */
    uint8_t GetMaintenanceValue(void) const;
    /**
    *
    *
    * \return
    */
    bool GetIsMaster(void) const;

private:
    bool m_unitIndex;
    uint8_t m_value;
    bool m_isMaster;

};

} // namespace ns3

#endif /* FIELDS_HEADERS_H */
