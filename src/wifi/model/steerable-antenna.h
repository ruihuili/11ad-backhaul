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

namespace ns3 {

/**
 * \brief Steerable Antenna functionality for mmWave Communications.
 */
class SteerableAntenna : public Object
{
public:
    SteerableAntenna();
    virtual ~SteerableAntenna();
    static TypeId GetTypeId (void);

    void SetHalfPowerBeamWidth(double beamwidth);
    void SetAntennas(uint8_t id);
    void SetSectors(uint8_t sectors);
    void SetBoresight(double awv);

    double GetHalfPowerBeamWidth(void) const;
    uint8_t GetAntennas(void);
    uint8_t GetSectors(void);
    double GetBoresight(void);
    virtual double GetMaxGainDbi(void) const = 0;

private:
    double m_halfPowerBeamWidth;	    /* Main Lobe Function (Half-power beam width) Theta(-3dB). */
    uint8_t m_antenna;			    /* Number of antennas. */
    uint8_t m_sectors;			    /* Number of antenna sectors. */
    double m_boresight;			    /* Direction of the antenna. */

};

} // namespace ns3

#endif /* STEERABLE_ANTENNA_H */
