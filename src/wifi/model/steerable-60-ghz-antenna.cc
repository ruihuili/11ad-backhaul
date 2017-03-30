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
#include "steerable-60-ghz-antenna.h"

#include <math.h>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("Steerable60GhzAntenna");
NS_OBJECT_ENSURE_REGISTERED(Steerable60GhzAntenna);

TypeId Steerable60GhzAntenna::GetTypeId (void)
{
    static TypeId tid = TypeId("ns3::Steerable60GhzAntenna")
	.SetParent<SteerableAntenna>()
	.SetConstructor(Steerable60GhzAntenna)
	;
    return tid;
}

Steerable60GhzAntenna::Steerable60GhzAntenna()
{}

Steerable60GhzAntenna::~Steerable60GhzAntenna()
{}

double Steerable60GhzAntenna::GetMaxGainDbi(void)
{
    double maxGain;
    maxGain = 10 * log10(pow(1.6162/sin(m_mainLobeFunction/2), 2));
    return maxGain;
}

double Steerable60GhzAntenna::GetSideLobeGain(void)
{
    double sideLobeGain;
    sideLobeGain = -0.4111 * log(m_mainLobeFunction) - 10.597;
    return sideLobeGain;
}

double Steerable60GhzAntenna::GetGainDbi(double angle)
{
    double gain;
    double m_mainLobeWidth;	    /* Main Lobe Width (The first zero) Theta(ml). */

    m_mainLobeWidth = 2.6 * m_halfPowerBeamWidth;

    if (0 <= angle <= m_halfPowerBeamWidth/2)
    {
	gain = GetMaxGainDbi() - 3.01 * pow(2 * angle/m_mainLobeFunction, 2);
    }
    else
    {
	gain = GetSideLobeGain();
    }
    return gain;
}

}
