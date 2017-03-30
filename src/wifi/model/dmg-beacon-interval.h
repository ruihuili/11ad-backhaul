/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2015 NICOLO FACCHI
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
 * Author: Nicolo' Facchi <nicolo.facchi@gmail.com>
           Rui Li <lirui628@gmail.com>
 */
#ifndef DMG_BEACON_INTERVAL_H
#define DMG_BEACON_INTERVAL_H

#include "ns3/object.h"
#include "ns3/nstime.h"
#include "ns3/mobility-model.h"
#include "ns3/mac48-address.h"
#include "ns3/ipv4-address.h"
#include "ns3/event-id.h"
#include "dmg-antenna-controller.h"
#include "wifi-mac-header.h"



namespace ns3 {

/* This class describe a Service Period */
class DmgServicePeriod : public Object
{
public:
  DmgServicePeriod ();
  virtual ~DmgServicePeriod ();

  /* Set the start time (relative to the beginning of the Beacon Interval) of
   * this Service Period */
  void SetSpStart (Time start);
  /* Return the start time (relative to the beginning of the Beacon Interval) of
   * this Service Period */
  Time GetSpStart (void);
  /* Set the stop time (relative to the beginning of the Beacon Interval) of
   * this Service Period */
  void SetSpStop (Time stop);
  /* Return the stop time (relative to the beginning of the Beacon Interval) of
   * this Service Period */
  Time GetSpStop (void);
  /* Set the MAC address of the *NEXT HOP* destination of this Service Period */
  void SetSpDestination (Mac48Address dest);
  /* Return the MAC address of the *NEXT HOP* destination of this Service Period */
  Mac48Address GetSpDestination (void);
  /* Set the Ipv4 address of the FLOW source and sink of this Service Period */
  void SetSpFlowSrcSinkIpv4Address (Ipv4Address src, Ipv4Address sink);

  /* Return the Ipv4 address of the *FLOW'S FINAL* destination of this Service Period */
  std::pair <Ipv4Address,Ipv4Address> GetSpFlowSourceSinkIpv4Address (void);

  /* Set Mobility model of the destination of this Service Period.
   * This is useful to know the position of the destination. In real life this
   * information is not availble.
   * */
  void SetSpDestinationMobility(Ptr<MobilityModel> mob);
  /* Return the mobility model of the destination of this Service Period */
  Ptr<MobilityModel> GetSpDestinationMobility (void);

  /* Get whether the station itself is transmitting during the SP*/
  bool GetSpIfTx(void);
  /* Set whether the station itself is transmitting during the SP*/
  void SetSpIfTx(bool);

  void SetBeamSwitchOverhead (Time beamSwitchOverhead);
  //Time GetBeamSwitchOverhead (void);

private:
  /* Start time of this Serive Period. The start time is relative to the
   * beginning of the current Beacon Interval */
  Time m_spStart;
  /* Stop time of this Serive Period. The stop time is relative to the
   * beginning of the current Beacon Interval.
   * m_spStop must be > m_spStart */
  Time m_spStop;
  /* Get whether the station itself is transmitting or receving in the SP -----added on 20 Jan*/
  bool m_spTransmitter;

  /* MAC address of the *NEXT HOP* destination of this Service Period */
  Mac48Address m_spDestination;

  /* Ipv4 address of the *FLOW'S FINAL* destination of this Service Period */
  std::pair <Ipv4Address, Ipv4Address> m_spFlowSourceSinkIpv4Address;

  /* Mobility model of the destination of this Service Period.
   * This info is not available in real. We use it for covenience
   */
  Ptr<MobilityModel> m_spDestinationMobility;

  Time m_beamSwitchOverhead;

};

/* This class represents a Beacon Interval.
 * In the current implementation it is very simplified. It only contains a list
 * of Service Periods. No other kind of transmission period are currently
 * allowed (e.g. it is not possible to mix CBAP to SP).
 */
class DmgBeaconInterval : public Object
{
public:
  DmgBeaconInterval ();

  virtual ~DmgBeaconInterval ();

  /* Set the duration of this Beacon Interval */
  void SetBiDuration (Time biDur);
  /* Return the duration of this Beacon Interval */
  Time GetBiDuration (void);
  /* Add a new Service Period to this Beacon Interval.
   * It is a caller responsibility to check that the Service Period added to
   * this Beacon Interval do not overlap to each other.
   * -----------------------------------The last attribut 'bool transmitt' is added on 20 Jan*/
  void AddSp (Time start, Time stop, Mac48Address dest, Ipv4Address srcIpv4 ,Ipv4Address sinkIpv4, Ptr<MobilityModel> mob, bool transmitt );
  /* Erase all the Service Periods of this beacon Interval */
  void EraseSp ();

  /* The following methods return the absolute start and end times of the SP +
   * the destination MAC.
   * The asbolute start time could be also in the past
   */
  /* Return the start time of the next (or current if the service period is not
   * finished yet. Service period. The start time is
   * returned as an absolute time.  
   * The returned start time could be also in the past if the Nodes this Beacon
   * Interval is associated with is in the middle
   * of a Service Period.
   */
  Time GetNextSpStart (void);
  /* Return the stop time of the next (or current if the service period is not
   * finished yet) service period.
   */
  Time GetNextSpStop (void);
    
  /* Get whether the station itself is transmitting in the next SP, return true if yes -----added on 20 Jan*/
  bool GetNextSpIfTx(void);
  Time GetNextTxSpStart (void);
  Time GetNextTxSpStop (void);
  Mac48Address GetNextTxSpDestination (void);
  std::pair <Ipv4Address, Ipv4Address> GetNextTxSpSrcSinkIpv4Address (void);
  Ptr<MobilityModel> GetNextTxSpDestinationMobility (void);
    
  /* Return the destination MAC address of the next (or current if the service period is not
   * finished yet) service period.
   */
  Mac48Address GetNextSpDestination (void);
  /* Return the destination mobility model of the next (or current if the service period is not
   * finished yet) service period.
   */
  Ptr<MobilityModel> GetNextSpDestinationMobility (void);
  /* Return the vector of all the Service Period associated with this Beacon
   * Interval
   */
  std::vector<Ptr<DmgServicePeriod> > GetSps(void);

  /* A beacon Interval knows the antenna controller of the node.
   * This is useful to force antenna alignment at the beginning of each service
   * period.
   */
  void SetDmgAntennaController (Ptr<DmgAntennaController> dmgBi);
  Ptr<DmgAntennaController> GetDmgAntennaController (void);

  /* Schedule the antenna alignment at the beginning of the next Service period
   * */
  void ScheduleNextAntennaAlignment ();
    
    void SetBeamSwitchOverhead (Time beamSwitchOverhead);
private:
  /* used by ScheduleNextAntennaAlignment */
  void AlignAntenna();

  /* Duration of this Beacon Interval */
  Time m_biDuration;
    
  /* List of Service Periods associated with this Beacon Interval */
  std::vector<Ptr<DmgServicePeriod> > m_sp;
  /* Antenna controller of the node this Beacon Interval is associated with */
  Ptr<DmgAntennaController> m_dmgAntennaController;

  /* support variables used to handle antenna alignments at the beginning of the
   * service periods */
  Time m_alignAntennaTime;
  EventId m_alignAntenna;
  uint32_t m_lastAlignAntennaSpIndex;
  bool m_initialized;
  Time m_beamSwitchOverhead;

};

} // namespace ns3

#endif /* DMG_BEACON_INTERVAL_H */
