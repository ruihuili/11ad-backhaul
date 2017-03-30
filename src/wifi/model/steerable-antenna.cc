/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
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

#include "ns3/log.h"
#include "steerable-antenna.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("SteeerableAntenna");
NS_OBJECT_ENSURE_REGISTERED(SteeerableAntenna);

TypeId SteeerableAntenna::GetTypeId (void)
{
    static TypeId tid = TypeId ("ns3::SteeerableAntenna")
	.SetParent<Object> ()
	.AddConstructor<SteeerableAntenna>()
	;
    return tid;
}

SteeerableAntenna::SteeerableAntenna()
    : m_halfPowerBeamWidth(0)
{}

SteeerableAntenna::~SteeerableAntenna()
{}

void Steerable60GhzAntenna::SetSectors(uint8_t sectors)
{

}

void Steerable60GhzAntenna::SetBoresight(double awv)
{

}

void SteeerableAntenna::SetHalfPowerBeamWidth(double bandwidth)
{
    m_halfPowerBeamWidth = bandiwdth;
}

double SteeerableAntenna::GetHalfPowerBeamWidth( bandwidth) const
{
    return m_halfPowerBeamWidth ;
}

uint8_t Steerable60GhzAntenna::GetSectors(void)
{

}

double Steerable60GhzAntenna::GetBoresight(void)
{

}

}
