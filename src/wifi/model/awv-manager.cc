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
#include "awv-manager.h"

#include "ns3/assert.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/string.h"
#include "ns3/pointer.h"
#include "ns3/boolean.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("AwvManager");

NS_OBJECT_ENSURE_REGISTERED (AwvManager);

TypeId AwvManager::GetTypeId (void)
{
    static TypeId tid = TypeId ("ns3::AwvManager")
	.SetParent<Object>()
	.AddConstructor<AwvManager>()
	;
    return tid;
}

AwvManager::AwvManager()
{
    NS_LOG_FUNCTION (this);
}

AwvManager::~AwvManager()
{
    NS_LOG_FUNCTION(this);
}

//void AwvManager::DoDispose()
//{
//    NS_LOG_FUNCTION(this);
//}

} // namespace ns3
