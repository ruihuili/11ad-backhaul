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
#ifndef AWV_MANAGER_H
#define AWV_MANAGER_H

#include "ns3/object.h"

namespace ns3 {

/**
 * \brief Wi-Fi Antenna Weight Vector Manager
 * \ingroup wifi
 *
 * The purpose of the AWV Manager is to provide the Beamforming Engine
 * with the values and decisions necessary for link training during the SLS,
 * BRP and Beam Tracking. It acts as an interface for the retrieval of various
 * learnt link parameters, the next parameters to test, and the next type of
 * test to perform. The AWV Manager maintains a list of all the STAs that
 * this STA has performed any beamforming procedure with.
 *
 */
class AwvManager : public Object
{
public:
    AwvManager();
    virtual ~AwvManager();
    static TypeId GetTypeId(void);

private:


};

} // namespace ns3

#endif /* AWV_MANAGER_H */
