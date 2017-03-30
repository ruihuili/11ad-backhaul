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
#ifndef DMG_AP_WIFI_MAC_H
#define DMG_AP_WIFI_MAC_H

#include "dmg-wifi-mac.h"

#include "amsdu-subframe-header.h"

namespace ns3 {

/**
 * \ingroup wifi
 *
 * This implements a DMG Wifi Mac for APs.
 *
 * In the current implementation there is no difference between DmgApWifiMac and
 * DmgStaWifiMac. They both work as Ad-Hoc stations.
 *
 * In future implementation DmgApWifiMac will implement features specific for AP
 * (e.g. beaconing management)
 *
 */
class DmgApWifiMac : public DmgWifiMac
{
public:
  static TypeId GetTypeId (void);

  DmgApWifiMac ();
  virtual ~DmgApWifiMac ();

};

} // namespace ns3

#endif /* DMG_AP_WIFI_MAC_H */
