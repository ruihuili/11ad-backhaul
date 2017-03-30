/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2015, HANY ASSASA
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
#ifndef BEAMFORMING_ENGINE_H
#define BEAMFORMING_ENGINE_H

#include "ns3/object.h"

namespace ns3 {

/**
 * \brief 802.11ad Beamforming Engine.
 * \ingroup wifi
 *
 * The Beamforming Engine performs the beamforming procedures and protocols
 * with the guidance of the AWV Manager.
 *
 */
class BeamformingEngine : public Object
{
public:
    BeamformingEngine();
    virtual ~BeamformingEngine();
    static TypeId GetTypeId(void);

private:

};

} // namespace ns3

#endif /* BEAMFORMING_ENGINE_H */
