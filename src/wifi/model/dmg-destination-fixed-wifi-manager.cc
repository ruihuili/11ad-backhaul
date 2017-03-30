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

#include "dmg-destination-fixed-wifi-manager.h"

#include "ns3/string.h"
#include "ns3/assert.h"
#include "ns3/log.h"

#define Min(a,b) ((a < b) ? a : b)

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("DmgDestinationFixedWifiManager");

NS_OBJECT_ENSURE_REGISTERED (DmgDestinationFixedWifiManager);

TypeId
DmgDestinationFixedWifiManager::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::DmgDestinationFixedWifiManager")
    .SetParent<WifiRemoteStationManager> ()
    .AddConstructor<DmgDestinationFixedWifiManager> ()
  ;
  return tid;
}

DmgDestinationFixedWifiManager::DmgDestinationFixedWifiManager ()
{
  NS_LOG_FUNCTION (this);
}

DmgDestinationFixedWifiManager::~DmgDestinationFixedWifiManager ()
{
  NS_LOG_FUNCTION (this);
}

WifiRemoteStation *
DmgDestinationFixedWifiManager::DoCreateStation (void) const
{
  NS_LOG_FUNCTION (this);
  WifiRemoteStation *station = new WifiRemoteStation ();
  return station;
}

void
DmgDestinationFixedWifiManager::DoReportRxOk (WifiRemoteStation *station,
                                       double rxSnr, WifiMode txMode)
{
  NS_LOG_FUNCTION (this << station << rxSnr << txMode);
}

void
DmgDestinationFixedWifiManager::DoReportRtsFailed (WifiRemoteStation *station)
{
  NS_LOG_FUNCTION (this << station);
}

void
DmgDestinationFixedWifiManager::DoReportDataFailed (WifiRemoteStation *station)
{
  NS_LOG_FUNCTION (this << station);
}

void
DmgDestinationFixedWifiManager::DoReportRtsOk (WifiRemoteStation *st,
                                        double ctsSnr, WifiMode ctsMode, double rtsSnr)
{
  NS_LOG_FUNCTION (this << st << ctsSnr << ctsMode << rtsSnr);
}

void
DmgDestinationFixedWifiManager::DoReportDataOk (WifiRemoteStation *st,
                                         double ackSnr, WifiMode ackMode, double dataSnr)
{
  NS_LOG_FUNCTION (this << st << ackSnr << ackMode << dataSnr);
}

void
DmgDestinationFixedWifiManager::DoReportFinalRtsFailed (WifiRemoteStation *station)
{
  NS_LOG_FUNCTION (this << station);
}

void
DmgDestinationFixedWifiManager::DoReportFinalDataFailed (WifiRemoteStation *station)
{
  NS_LOG_FUNCTION (this << station);
}

WifiTxVector
DmgDestinationFixedWifiManager::DoGetDataTxVector (WifiRemoteStation *st, uint32_t size)
{
  NS_LOG_FUNCTION (this << st << size);
  std::map<Mac48Address, WifiMode>::iterator it =
	  m_modePerDestination.find(st->m_state->m_address);
  NS_ASSERT(it != m_modePerDestination.end());
  WifiMode mode = it->second;
  return WifiTxVector (mode, GetDefaultTxPowerLevel (), GetLongRetryCount (st),
		       GetShortGuardInterval (st),
		       Min (GetNumberOfReceiveAntennas (st),
			    GetNumberOfTransmitAntennas()),
		       GetNess (st), GetStbc (st));
}

WifiMode
DmgDestinationFixedWifiManager::GetControlAnswerMode (Mac48Address address, WifiMode
                                               reqMode)
{
  return reqMode;
}

WifiTxVector
DmgDestinationFixedWifiManager::DoGetRtsTxVector (WifiRemoteStation *st)
{
  NS_LOG_FUNCTION (this << st);
  NS_ASSERT(false);
  /*
  return WifiTxVector (m_ctlMode, GetDefaultTxPowerLevel (),
  GetShortRetryCount (st), GetShortGuardInterval (st),
  Min (GetNumberOfReceiveAntennas (st),GetNumberOfTransmitAntennas()),
  GetNess (st), GetStbc (st));
  */
}

bool
DmgDestinationFixedWifiManager::IsLowLatency (void) const
{
  NS_LOG_FUNCTION (this);
  return true;
}

void
DmgDestinationFixedWifiManager::AddDestinationWifiMode (Mac48Address mac, WifiMode mode)
{
  std::map<Mac48Address, WifiMode>::iterator it =
	  m_modePerDestination.find(mac);

  if (it != m_modePerDestination.end()) {
    m_modePerDestination.erase(it);
  }

  m_modePerDestination.insert(
	std::pair<Mac48Address, WifiMode>(mac, mode));
}
//----------------------------added on 17.02.2016 Edinburgh
WifiMode
DmgDestinationFixedWifiManager::GetDestinationWifiMode (Mac48Address mac)
{
    std::map<Mac48Address, WifiMode>::iterator it =
    m_modePerDestination.find(mac);
    
    NS_ASSERT(it != m_modePerDestination.end());

    return m_modePerDestination.at(mac);
} //----------------------------added on 17.02.2016 Edinburgh

void
DmgDestinationFixedWifiManager::DeleteDestinationsMap (void)
{
  m_modePerDestination.clear();
}

} // namespace ns3
