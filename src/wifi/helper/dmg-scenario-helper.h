/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2015 NICOLO FACCHI
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
#ifndef DMG_SCENARIO_HELPER_H
#define DMG_SCENARIO_HELPER_H

#include "wifi-helper.h"
#include "ns3/qos-utils.h"

namespace ns3 {

class DmgScenarioHelper
{
public:
  DmgScenarioHelper ();

  virtual ~DmgScenarioHelper ();


 };

} // namespace ns3

#endif /* DMG_SCENARIO_HELPER_H */
