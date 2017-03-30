/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
//
// Copyright (c) 2006 Georgia Tech Research Corporation
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation;
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
// Author: George F. Riley<riley@ece.gatech.edu>
//

// ns3 - On/Off Data Source Application class
// George F. Riley, Georgia Tech, Spring 2007
// Adapted from ApplicationDashRateAdaptation in GTNetS.

#include "ns3/log.h"
#include "ns3/address.h"
#include "ns3/inet-socket-address.h"
#include "ns3/inet6-socket-address.h"
#include "ns3/packet-socket-address.h"
#include "ns3/node.h"
#include "ns3/nstime.h"
#include "ns3/data-rate.h"
#include "ns3/random-variable-stream.h"
#include "ns3/socket.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"
#include "ns3/trace-source-accessor.h"
#include "dash-rate-adaptation-application.h"
#include "ns3/udp-socket-factory.h"
#include "ns3/string.h"
#include "ns3/pointer.h"
#include "seq-ts-header.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("DashRateAdaptationApplication");

NS_OBJECT_ENSURE_REGISTERED (DashRateAdaptationApplication);

TypeId
DashRateAdaptationApplication::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::DashRateAdaptationApplication")
    .SetParent<Application> ()
    .AddConstructor<DashRateAdaptationApplication> ()
    .AddAttribute ("Interval", "The time interval between data packet tx during on state in nano seconds",
                   StringValue ("ns3::ConstantRandomVariable[Constant=1.0]"),
                   MakePointerAccessor (&DashRateAdaptationApplication::m_vbrInterval),
                   MakePointerChecker <RandomVariableStream>())
                   /*"DataRate", "The data rate in on state.",
                   DataRateValue (DataRate ("500kb/s")),
                   MakeDataRateAccessor (&DashRateAdaptationApplication::m_cbrRate),
                   MakeDataRateChecker ())*/
    .AddAttribute ("DataRateProbInterval", "The probability interval from which the data rate is chosen.",
                   StringValue ("ns3::ConstantRandomVariable[Constant=0.5]"),//defalt the median 
                   MakePointerAccessor (&DashRateAdaptationApplication::m_vbrRateProbInterval),
                   MakePointerChecker <RandomVariableStream>())
/*                 "PacketSize", "The size of packets sent in on state",
                   StringValue ("ns3::ConstantRandomVariable[Constant=512.0]"),
                   MakePointerAccessor (&DashRateAdaptationApplication::m_vbrPktSize),
                   MakePointerChecker <RandomVariableStream>())*/
/*                   UintegerValue (512),
                   MakeUintegerAccessor (&DashRateAdaptationApplication::m_pktSize),
                   MakeUintegerChecker<uint32_t> (1))*/
    .AddAttribute ("Remote", "The address of the destination",
                   AddressValue (),
                   MakeAddressAccessor (&DashRateAdaptationApplication::m_peer),
                   MakeAddressChecker ())
    .AddAttribute ("OnTime", "A RandomVariableStream used to pick the duration of the 'On' state.",
                   StringValue ("ns3::ConstantRandomVariable[Constant=1.0]"),
                   MakePointerAccessor (&DashRateAdaptationApplication::m_onTime),
                   MakePointerChecker <RandomVariableStream>())
    .AddAttribute ("OffTime", "A RandomVariableStream used to pick the duration of the 'Off' state.",
                   StringValue ("ns3::ConstantRandomVariable[Constant=1.0]"),
                   MakePointerAccessor (&DashRateAdaptationApplication::m_offTime),
                   MakePointerChecker <RandomVariableStream>())
    .AddAttribute ("MaxBytes", 
                   "The total number of bytes to send. Once these bytes are sent, "
                   "no packet is sent again, even in on state. The value zero means "
                   "that there is no limit.",
                   UintegerValue (0),
                   MakeUintegerAccessor (&DashRateAdaptationApplication::m_maxBytes),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("Protocol", "The type of protocol to use.",
                   TypeIdValue (UdpSocketFactory::GetTypeId ()),
                   MakeTypeIdAccessor (&DashRateAdaptationApplication::m_tid),
                   MakeTypeIdChecker ())
    .AddTraceSource ("Tx", "A new packet is created and is sent",
                     MakeTraceSourceAccessor (&DashRateAdaptationApplication::m_txTrace),
                     "ns3::Packet::TracedCallback")
  ;
  return tid;
}


DashRateAdaptationApplication::DashRateAdaptationApplication ()
  : m_socket (0),
    m_sent(0),
    m_connected (false),
    m_residualBits (0),
    m_lastStartTime (Seconds (0)),
    m_totBytes (0),
    m_pktSize (512)
{
  NS_LOG_FUNCTION (this);
}

DashRateAdaptationApplication::~DashRateAdaptationApplication()
{
  NS_LOG_FUNCTION (this);
}

void 
DashRateAdaptationApplication::SetMaxBytes (uint32_t maxBytes)
{
  NS_LOG_FUNCTION (this << maxBytes);
  m_maxBytes = maxBytes;
}

Ptr<Socket>
DashRateAdaptationApplication::GetSocket (void) const
{
  NS_LOG_FUNCTION (this);
  return m_socket;
}

int64_t 
DashRateAdaptationApplication::AssignStreams (int64_t stream)
{
  NS_LOG_FUNCTION (this << stream);
  m_onTime->SetStream (stream);
  m_offTime->SetStream (stream + 1);
  m_vbrInterval->SetStream (stream + 2);
  m_vbrRateProbInterval->SetStream (stream + 3);
  return 4;
}

void
DashRateAdaptationApplication::DoDispose (void)
{
  NS_LOG_FUNCTION (this);

  m_socket = 0;
  // chain up
  Application::DoDispose ();
}

// Application Methods
void DashRateAdaptationApplication::StartApplication () // Called at time specified by Start
{
  NS_LOG_FUNCTION (this);

  // Create the socket if not already
  if (!m_socket)
    {
      m_socket = Socket::CreateSocket (GetNode (), m_tid);
      if (Inet6SocketAddress::IsMatchingType (m_peer))
        {
          m_socket->Bind6 ();
        }
      else if (InetSocketAddress::IsMatchingType (m_peer) ||
               PacketSocketAddress::IsMatchingType (m_peer))
        {
          m_socket->Bind ();
        }
      m_socket->Connect (m_peer);
      m_socket->SetAllowBroadcast (true);
      m_socket->ShutdownRecv ();

      m_socket->SetConnectCallback (
        MakeCallback (&DashRateAdaptationApplication::ConnectionSucceeded, this),
        MakeCallback (&DashRateAdaptationApplication::ConnectionFailed, this));
    }
  m_vbrIntervalFailSafe = m_vbrInterval;

  // Insure no pending event
  CancelEvents ();
  // If we are not yet connected, there is nothing to do here
  // The ConnectionComplete upcall will start timers at that time
  //if (!m_connected) return;
  ScheduleStartEvent ();
}

void DashRateAdaptationApplication::StopApplication () // Called at time specified by Stop
{
  NS_LOG_FUNCTION (this);

  CancelEvents ();
  if(m_socket != 0)
    {
      m_socket->Close ();
    }
  else
    {
      NS_LOG_WARN ("DashRateAdaptationApplication found null socket to close in StopApplication");
    }
}

void DashRateAdaptationApplication::CancelEvents ()
{
  NS_LOG_FUNCTION (this);

  /*if (m_sendEvent.IsRunning () && m_vbrIntervalFailSafe == m_vbrInterval )
    { // Cancel the pending send packet event
      // Calculate residual bits since last packet sent
      Time delta (Simulator::Now () - m_lastStartTime);
      int64x64_t bits = delta.To (Time::S) * (m_vbrInterval);
      m_residualBits += bits.GetHigh ();
    }
  m_vbrIntervalFailSafe = m_vbrInterval;*/
  Simulator::Cancel (m_sendEvent);
  Simulator::Cancel (m_startStopEvent);
}

// Event handlers
void DashRateAdaptationApplication::StartSending ()
{
  NS_LOG_FUNCTION (this);
  m_lastStartTime = Simulator::Now ();
  ScheduleNextTx ();  // Schedule the send packet event
  ScheduleStopEvent ();
}

void DashRateAdaptationApplication::StopSending ()
{
  NS_LOG_FUNCTION (this);
  CancelEvents ();

  ScheduleStartEvent ();
}

// Private helpers
void DashRateAdaptationApplication::ScheduleNextTx ()
{
  NS_LOG_FUNCTION (this);

  if (m_maxBytes == 0 || m_totBytes < m_maxBytes)
    {

      uint64_t interval = m_vbrInterval->GetValue ();
      m_pktSize = interval * MapBitRate() / 1e9 /8;

      Time nextTime (NanoSeconds (interval));

      NS_LOG_LOGIC ("nextTime = " << nextTime <<" pkt size " << m_pktSize <<" bytes ");
      m_sendEvent = Simulator::Schedule (nextTime,
                                         &DashRateAdaptationApplication::SendPacket, this);
    }
  else
    { // All done, cancel any pending events
      StopApplication ();
    }
}

uint32_t DashRateAdaptationApplication::MapBitRate ()
{
  NS_LOG_FUNCTION (this);
  uint64_t rate;
  //unified random distribution()

  double prob = m_vbrRateProbInterval->GetValue ();
 
  std::map <uint32_t, double>::const_iterator it = m_rateMap.begin();

  double lowEdge = it->second;
  //map iterator
  while (it != m_rateMap.end())
  {
        if ( prob >= lowEdge && prob < (it->second + lowEdge) )
        {
                rate = it->first;
                break;
        }

        lowEdge += it->second;
        it++;
  }

  NS_LOG_INFO(" Prob (rand val): " << prob << " mapped rate "<< rate);
  return rate;
}

void DashRateAdaptationApplication::SetBitRateMap (std::map<uint32_t, double> prob)
{
  NS_LOG_FUNCTION (this);
  m_rateMap = prob;
}

void DashRateAdaptationApplication::ScheduleStartEvent ()
{  // Schedules the event to start sending data (switch to the "On" state)
  NS_LOG_FUNCTION (this);

  Time offInterval = Seconds (m_offTime->GetValue ());
  NS_LOG_LOGIC ("start at " << offInterval);
  m_startStopEvent = Simulator::Schedule (offInterval, &DashRateAdaptationApplication::StartSending, this);
}

void DashRateAdaptationApplication::ScheduleStopEvent ()
{  // Schedules the event to stop sending data (switch to "Off" state)
  NS_LOG_FUNCTION (this);

  Time onInterval = Seconds (m_onTime->GetValue ());
  NS_LOG_LOGIC ("stop at " << onInterval);
  m_startStopEvent = Simulator::Schedule (onInterval, &DashRateAdaptationApplication::StopSending, this);
}


void DashRateAdaptationApplication::SendPacket ()
{
  NS_LOG_FUNCTION (this);

  NS_ASSERT (m_sendEvent.IsExpired ());

  Ptr<Packet> packet;

  NS_LOG_INFO("APP TID: "<< m_tid);

  if(m_tid.GetName() == "ns3::UdpSocketFactory")
  {
            SeqTsHeader seqTs;
            seqTs.SetSeq (m_sent);
            packet = Create<Packet> (m_pktSize-(8+4));
            packet->AddHeader (seqTs);
  }
  else if (m_tid.GetName() == "ns3::TcpSocketFactory")
  {
            packet = Create<Packet> (m_pktSize-(8+4));
  }
  //NS_LOG_UNCOND("APP packet size "<<packet->GetSize());
  //NS_LOG_UNCOND("seqTs "<< seqTs.GetSeq());
  m_txTrace (packet);
  //m_socket->Send (packet);
  if ((m_socket->Send (packet)) >= 0){
        ++m_sent;
  }
  else{
        NS_LOG_INFO ("Error while sending "<<  packet->GetSize () << " bytes to "
                     << InetSocketAddress::ConvertFrom(m_peer).GetIpv4 ());
  }
  m_totBytes += m_pktSize;
                     
  if (InetSocketAddress::IsMatchingType (m_peer))
    {
      NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds ()
                   << "s application sent "
                   <<  packet->GetSize () << " bytes to "
                   << InetSocketAddress::ConvertFrom(m_peer).GetIpv4 ()
                   << " port " << InetSocketAddress::ConvertFrom (m_peer).GetPort ()
                   << " total Tx " << m_totBytes << " bytes");
    }
  else if (Inet6SocketAddress::IsMatchingType (m_peer))
    {
      NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds ()
                   << "s application sent "
                   <<  packet->GetSize () << " bytes to "
                   << Inet6SocketAddress::ConvertFrom(m_peer).GetIpv6 ()
                   << " port " << Inet6SocketAddress::ConvertFrom (m_peer).GetPort ()
                   << " total Tx " << m_totBytes << " bytes");
    }

  m_lastStartTime = Simulator::Now ();
  m_residualBits = 0;
  ScheduleNextTx ();
}


void DashRateAdaptationApplication::ConnectionSucceeded (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
  m_connected = true;
}

void DashRateAdaptationApplication::ConnectionFailed (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
}


uint64_t 
DashRateAdaptationApplication::GetTotalTxBytes (void)
{
  NS_LOG_FUNCTION (this);
  return m_totBytes;
}

} // Namespace ns3
