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

#include "dmg-antenna-controller.h"
#include "ns3/yans-wifi-phy.h"
#include "ns3/mobility-model.h"
#include "ns3/log.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("DmgAntennaController");

NS_OBJECT_ENSURE_REGISTERED (DmgAntennaController);

DmgAntennaController::DmgAntennaController ()
{
}

DmgAntennaController::~DmgAntennaController ()
{
}

void
DmgAntennaController::SetPhy (Ptr<WifiPhy> phy)
{
  m_phy = phy;
}

void
DmgAntennaController::SetAntenna (Ptr<AbstractAntenna> antenna)
{
  m_antenna = antenna;
}

double
DmgAntennaController::GetTxGainDbi(double azimuth, double elevation)
{
  return m_antenna->GetTxGainDbi(azimuth, elevation);
}

double
DmgAntennaController::GetRxGainDbi(double azimuth, double elevation)
{
  return m_antenna->GetRxGainDbi(azimuth, elevation);
}

void
DmgAntennaController::PointAntenna (Ptr<WifiPhy> target)
{
  Vector sourcePos = m_phy->GetObject<YansWifiPhy>()->
	  GetMobility()->GetObject<MobilityModel>()->GetPosition();
  Vector targetPos = target->GetObject<YansWifiPhy>()->
	  GetMobility()->GetObject<MobilityModel>()->GetPosition();

  double azimuthAngle = atan2(targetPos.y - sourcePos.y,
			      targetPos.x - sourcePos.x);

  // Change the azimuth angle of the antenna to point to targer.
  // We assume elevation in 0.
  m_antenna->SetAzimuthAngle(azimuthAngle);
}

void
DmgAntennaController::PointAntenna (Vector targetPos)
{
  Vector sourcePos = m_phy->GetObject<YansWifiPhy>()->
	  GetMobility()->GetObject<MobilityModel>()->GetPosition();

  double azimuthAngle = atan2(targetPos.y - sourcePos.y,
			      targetPos.x - sourcePos.x);

  // Change the azimuth angle of the antenna to point to targerPos.
  // We assume elevation in 0.
  m_antenna->SetAzimuthAngle(azimuthAngle);
}

double
DmgAntennaController::GetBeamwidthDegrees (void)
{
  return m_antenna->GetBeamwidthDegrees();
}

} // namespace ns3

