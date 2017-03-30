/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2015 NICOLO' FACCHI 
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
 */
#ifndef DMG_DESTINATION_FIXED_WIFI_MANAGER_H
#define DMG_DESTINATION_FIXED_WIFI_MANAGER_H

#include <stdint.h>

#include "wifi-remote-station-manager.h"
#include <map>
#include "ns3/mac48-address.h"

namespace ns3 {

/* This WifiManager maintains a maps that associates to each possible mac
 * destination address an MCS
 */
class DmgDestinationFixedWifiManager : public WifiRemoteStationManager
{
public:
  static TypeId GetTypeId (void);
  DmgDestinationFixedWifiManager ();
  virtual ~DmgDestinationFixedWifiManager ();

  /* Add a destination MAC address and associated with the the WifiMode mode
   * Every frame transmitted to mac will be transmitted with the WifiMode mode.*/
  void AddDestinationWifiMode (Mac48Address mac, WifiMode mode);
    
  //----------------------------added on 17.02.2016 Edinburgh
  /* a simple utility function that can return the WifiMode that is employed 
   between a node and its destination*/
  WifiMode GetDestinationWifiMode (Mac48Address mac);
  //----------------------------added on 17.02.2016 Edinburgh
  /* Delete all the pairs destination MAC - WifiMode */
  void DeleteDestinationsMap (void);

private:
  // overriden from base class
  virtual WifiRemoteStation* DoCreateStation (void) const;
  virtual void DoReportRxOk (WifiRemoteStation *station,
                             double rxSnr, WifiMode txMode);
  virtual void DoReportRtsFailed (WifiRemoteStation *station);
  virtual void DoReportDataFailed (WifiRemoteStation *station);
  virtual void DoReportRtsOk (WifiRemoteStation *station,
                              double ctsSnr, WifiMode ctsMode, double rtsSnr);
  virtual void DoReportDataOk (WifiRemoteStation *station,
                               double ackSnr, WifiMode ackMode, double dataSnr);
  virtual void DoReportFinalRtsFailed (WifiRemoteStation *station);
  virtual void DoReportFinalDataFailed (WifiRemoteStation *station);

  /* Return the WifiTxVector (WifiMode) associated to the MAC address of the
   * WifiRemoteStation station */
  virtual WifiTxVector DoGetDataTxVector (WifiRemoteStation *station, uint32_t size);
  virtual WifiTxVector DoGetRtsTxVector (WifiRemoteStation *station);
  virtual bool IsLowLatency (void) const;

  /* With this manager the control (Ack) mode is always equal to the request
   * (data) mode.
   */
  virtual WifiMode GetControlAnswerMode (Mac48Address address, WifiMode
                                         reqMode);


  /* WifiMode map. Associated a WifiMode to each MAC destination address */
  std::map<Mac48Address, WifiMode> m_modePerDestination;

};

} // namespace ns3



#endif /* DMG_DESTINATION_FIXED_WIFI_MANAGER_H */
