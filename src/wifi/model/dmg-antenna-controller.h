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
#ifndef DMG_ANTENNA_CONTROLLER_H
#define DMG_ANTENNA_CONTROLLER_H

#include "ns3/object.h"
#include "ns3/wifi-phy.h"
#include "ns3/abstract-antenna.h"
#include "ns3/vector.h"

namespace ns3 {

/* At the moment DmgAntennaController assumes that the node phy is a YansWifiPhy
 * and that the node antenna is a ConeAntenna.
 * It also assumes that all the nodes are on the same plane (antenna elevation
 * is always 0)
 */
class DmgAntennaController : public Object
{
public:
  DmgAntennaController ();

  virtual ~DmgAntennaController ();

  /* Set the PHY of this node. 
   * In the current implementation we assume the PHY is a 
   * YansWifiPhy
   */
  void SetPhy (Ptr<WifiPhy> phy);
  /* Set the antenna of the node.
   * In the current implementation we assume the antenna is a
   * ConeAntenna
   */
  void SetAntenna (Ptr<AbstractAntenna> antenna);

  /* Return the transmission gain of the antenna for a given azimuth and
   * elevation.
   */
  double GetTxGainDbi(double azimuth, double elevation);
  /* Return the reception gain of the antenna for a given azimuth and
   * elevation.
   * In the current implementation we assume the ConeAntenna has the same gain
   * for both transmission and reception.
   */
  double GetRxGainDbi(double azimuth, double elevation);
  /* Return the beamwidth of the antenna.
   * With ConeAntenna the beamwidth depends on the Tx and Rx gain.
   */
  double GetBeamwidthDegrees (void);

  /* Change the azimuth of the antenna to point the target.
   * We assume elevation is always 0.
   * We assume target is a YansWifiPhy
   */
  void PointAntenna (Ptr<WifiPhy> target);
  /* Change the azimuth of the antenna to point the target whose position is
   * targetPos.
   * We assume elevation is always 0.
   */
  void PointAntenna (Vector targetPos);

private:

  /* PHY of the node this antenna controller is associated with.
   * We assume that m_phy is a YansWifiPhy.
   */
  Ptr<WifiPhy> m_phy;
  /* Antenna this antenna controller is controlling.
   * We assume m_antenna is a ConeAntenna
   */
  Ptr<AbstractAntenna> m_antenna;

};

} // namespace ns3

#endif /* DMG_ANTENNA_CONTROLLER_H */
