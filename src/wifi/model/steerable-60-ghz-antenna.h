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

#ifndef STEERABLE_ANTENNA_H
#define STEERABLE_ANTENNA_H

#include "ns3/object.h"
#include "steerable-antenna.h"

namespace ns3 {

/**
 * \brief Steerable Antenna functionality for 60 GHz based on IEEE 802.15.3c
 */
class Steerable60GhzAntenna : public SteerableAntenna
{
public:
    Steerable60GhzAntenna();
    ~Steerable60GhzAntenna();
    static TypeId GetTypeId (void);

    void SetSectors(uint8_t sectors);
    void SetBoresight(double awv);
    void SetMaxTxGainDbi(double gain);
    void SetMaxRxGainDbi(double gain);

    uint8_t GetSectors(void);
    double GetBoresight(void);

    double GetMaxGainDbi(void);
    double GetTxGainDbi(double angle);

    void SetHalfPowerBeamWidth(double bandwidth);
    double GetHalfPowerBeamWidth(void);
    double GetSideLobeGain(void);

};

} // namespace ns3

#endif /* STEERABLE_ANTENNA_H */
