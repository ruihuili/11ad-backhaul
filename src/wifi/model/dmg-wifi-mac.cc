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
#include "dmg-wifi-mac.h"

#include "ns3/log.h"
#include "ns3/boolean.h"
#include "ns3/pointer.h"
#include "ns3/uinteger.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/llc-snap-header.h"
#include "ns3/simulator.h"
#include "mac-rx-middle.h"
#include "mac-tx-middle.h"
#include "mac-low.h"
#include "dcf.h"
#include "dcf-manager.h"
#include "wifi-phy.h"
#include "yans-wifi-phy.h"
#include "wifi-mac-header.h"
#include "wifi-mac-trailer.h"
#include "ns3/ipv4-address.h"
#include "ns3/ipv4-header.h"
#include "msdu-aggregator.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("DmgWifiMac");

NS_OBJECT_ENSURE_REGISTERED(DmgWifiMac);

TypeId DmgWifiMac::GetTypeId (void)
{
    static TypeId tid = TypeId ("ns3::DmgWifiMac")
	.SetParent<WifiMac> ()
	.AddConstructor<DmgWifiMac> ()
	.AddAttribute ("QosSupported",
		       "This Boolean attribute is set to enable 802.11e/WMM-style QoS support at this STA",
		       BooleanValue (true),
		       MakeBooleanAccessor (&DmgWifiMac::SetQosSupported,
					    &DmgWifiMac::GetQosSupported),
		       MakeBooleanChecker ())
	.AddAttribute ("HtSupported",
		       "This Boolean attribute is set to enable 802.11n support at this STA",
		       BooleanValue (true),
		       MakeBooleanAccessor (&DmgWifiMac::SetHtSupported,
					    &DmgWifiMac::GetHtSupported),
		       MakeBooleanChecker ())
       .AddAttribute ("CtsToSelfSupported",
		       "Use CTS to Self when using a rate that is not in the basic set rate",
		       BooleanValue (false),
		       MakeBooleanAccessor (&DmgWifiMac::SetCtsToSelfSupported,
					    &DmgWifiMac::GetCtsToSelfSupported),
			MakeBooleanChecker ())
	.AddAttribute ("DcaTxop", "The DcaTxop object",
		       PointerValue (),
		       MakePointerAccessor (&DmgWifiMac::GetDcaTxop),
		       MakePointerChecker<DcaTxop> ())
	.AddAttribute ("VO_EdcaTxopN",
		       "Queue that manages packets belonging to AC_VO access class",
		       PointerValue (),
		       MakePointerAccessor (&DmgWifiMac::GetVOQueue),
		       MakePointerChecker<EdcaTxopN> ())
	.AddAttribute ("VI_EdcaTxopN",
		       "Queue that manages packets belonging to AC_VI access class",
		       PointerValue (),
		       MakePointerAccessor (&DmgWifiMac::GetVIQueue),
		       MakePointerChecker<EdcaTxopN> ())
	.AddAttribute ("BE_EdcaTxopN",
		       "Queue that manages packets belonging to AC_BE access class",
		       PointerValue (),
		       MakePointerAccessor (&DmgWifiMac::GetBEQueue),
		       MakePointerChecker<EdcaTxopN> ())
	.AddAttribute ("BK_EdcaTxopN",
		       "Queue that manages packets belonging to AC_BK access class",
		       PointerValue (),
		       MakePointerAccessor (&DmgWifiMac::GetBKQueue),
		       MakePointerChecker<EdcaTxopN> ())
	.AddTraceSource ( "TxOkHeader",
			  "The header of successfully transmitted packet",
			 MakeTraceSourceAccessor (&DmgWifiMac::m_txOkCallback),
			 "ns3::WifiMacHeader::TracedCallback")
	.AddTraceSource ("TxErrHeader",
			 "The header of unsuccessfully transmitted packet",
			 MakeTraceSourceAccessor (&DmgWifiMac::m_txErrCallback),
			 "ns3::WifiMacHeader::TracedCallback")
  .AddAttribute ("DmgAckTimeoutGuard", "Guard time for ack timeout (only if m_isDmg is true)",
                   TimeValue (MicroSeconds (1)),
                   MakeTimeAccessor (&DmgWifiMac::m_dmgAckTimeoutGuard),
                   MakeTimeChecker ())
    ;

    return tid;
}

DmgWifiMac::DmgWifiMac()
{
    NS_LOG_FUNCTION (this);

    m_rxMiddle = new MacRxMiddle();
    m_rxMiddle->SetForwardCallback(MakeCallback(&DmgWifiMac::Receive, this));

    m_txMiddle = new MacTxMiddle();

    /* Create Beamforming Engine */
    // m_beamformingEngine = CreateObject<BeamformingEngine>();

    m_low = CreateObject<MacLow>();
    m_low->SetRxCallback(MakeCallback(&MacRxMiddle::Receive, m_rxMiddle));
    m_low->SetIsDmg(true);

    m_dcfManager = new DcfManager();
    m_dcfManager->SetupLowListener(m_low);
    m_dcfManager->SetIsDmg(true);

    m_dmgBeaconInterval = 0;

    /*
    m_dca = CreateObject<DcaTxop>();
    m_dca->SetLow(m_low);
    m_dca->SetManager(m_dcfManager);
    m_dca->SetTxMiddle(m_txMiddle);
    m_dca->SetTxOkCallback(MakeCallback(&DmgWifiMac::TxOk, this));
    m_dca->SetTxFailedCallback(MakeCallback(&DmgWifiMac::TxFailed, this));
    */

    // Construct the EDCAFs. The ordering is important - highest
    // priority (Table 9-1 UP-to-AC mapping; IEEE 802.11-2012) must be created
    // first.
    //SetupEdcaQueue(AC_VO);
    //SetupEdcaQueue(AC_VI);
    SetupEdcaQueue(AC_BE);
    //SetupEdcaQueue(AC_BK);

    SetTypeOfStation (ADHOC_STA);
}

DmgWifiMac::~DmgWifiMac()
{
    NS_LOG_FUNCTION (this);
}

void DmgWifiMac::DoInitialize()
{
    NS_LOG_FUNCTION (this);
    //m_dca->Initialize();

    m_low->SetDmgAckTimeoutGuard(m_dmgAckTimeoutGuard);
    for (EdcaQueues::iterator i = m_edca.begin(); i != m_edca.end(); ++i)
    {
	i->second->Initialize();
    i->second->StartDmgSpTracking();
    }
}

void DmgWifiMac::DoDispose ()
{
    NS_LOG_FUNCTION (this);
    delete m_rxMiddle;
    m_rxMiddle = 0;

    delete m_txMiddle;
    m_txMiddle = 0;

    delete m_dcfManager;
    m_dcfManager = 0;

    m_low->Dispose ();
    m_low = 0;

    m_phy = 0;
    m_stationManager = 0;

    //m_dca->Dispose();
    //m_dca = 0;

    for (EdcaQueues::iterator i = m_edca.begin (); i != m_edca.end (); ++i)
    {
	i->second = 0;
    }
}

void DmgWifiMac::SetWifiRemoteStationManager(Ptr<WifiRemoteStationManager> stationManager)
{
    NS_LOG_FUNCTION(this << stationManager);
    m_stationManager = stationManager;
    m_stationManager->SetHtSupported(GetHtSupported());
    m_low->SetWifiRemoteStationManager(stationManager);

    //m_dca->SetWifiRemoteStationManager (stationManager);

    for (EdcaQueues::iterator i = m_edca.begin (); i != m_edca.end (); ++i)
    {
	i->second->SetWifiRemoteStationManager (stationManager);
    }
}

Ptr<WifiRemoteStationManager> DmgWifiMac::GetWifiRemoteStationManager () const
{
    return m_stationManager;
}

void DmgWifiMac::SetupEdcaQueue (enum AcIndex ac)
{
    NS_LOG_FUNCTION(this << ac);

    // Our caller shouldn't be attempting to setup a queue that is
    // already configured.
    NS_ASSERT(m_edca.find(ac) == m_edca.end());

    Ptr<EdcaTxopN> edca = CreateObject<EdcaTxopN>();
    edca->SetLow (m_low);
    edca->SetManager(m_dcfManager);
    edca->SetTxMiddle(m_txMiddle);
    edca->SetTxOkCallback(MakeCallback(&DmgWifiMac::TxOk, this));
    edca->SetTxFailedCallback(MakeCallback(&DmgWifiMac::TxFailed, this));
    edca->SetAccessCategory(ac);
    edca->SetIsDmg(true);
    edca->CompleteConfig();
    m_edca.insert(std::make_pair(ac, edca));
}

void DmgWifiMac::SetTypeOfStation(TypeOfStation type)
{
    NS_LOG_FUNCTION (this << type);
    for (EdcaQueues::iterator i = m_edca.begin (); i != m_edca.end (); ++i)
    {
	i->second->SetTypeOfStation(type);
    }
}

Ptr<DcaTxop> DmgWifiMac::GetDcaTxop() const
{
    return m_dca;
}

Ptr<EdcaTxopN> DmgWifiMac::GetVOQueue() const
{
    return m_edca.find (AC_VO)->second;
}

Ptr<EdcaTxopN> DmgWifiMac::GetVIQueue() const
{
    return m_edca.find (AC_VI)->second;
}

Ptr<EdcaTxopN> DmgWifiMac::GetBEQueue() const
{
    return m_edca.find (AC_BE)->second;
}

Ptr<EdcaTxopN> DmgWifiMac::GetBKQueue() const
{
    return m_edca.find (AC_BK)->second;
}

void DmgWifiMac::SetWifiPhy(Ptr<WifiPhy> phy)
{
    NS_LOG_FUNCTION (this << phy);
    m_phy = phy;
    m_dcfManager->SetupPhyListener (phy);
    m_low->SetPhy (phy);
}

Ptr<WifiPhy> DmgWifiMac::GetWifiPhy(void) const
{
    NS_LOG_FUNCTION (this);
    return m_phy;
}

void DmgWifiMac::ResetWifiPhy(void)
{
    NS_LOG_FUNCTION(this);
    m_low->ResetPhy();
    m_dcfManager->RemovePhyListener(m_phy);
    m_phy = 0;
}

void DmgWifiMac::SetForwardUpCallback(ForwardUpCallback upCallback)
{
    NS_LOG_FUNCTION (this);
    m_forwardUp = upCallback;
}

void DmgWifiMac::Enqueue (Ptr<const Packet> packet, Mac48Address to)
{
    NS_LOG_FUNCTION (this << packet << to);

    // for flow rate measurement
    Ptr<Packet> pCopy = packet->Copy ();
    NS_LOG_DEBUG (Simulator::Now ()<< " enqueue size " << packet->GetSize());// + hdr.GetSize()+ WIFI_MAC_FCS_LENGTH); //MPDU tags (4bytes not inclueded)

    LlcSnapHeader llc;
    pCopy->RemoveHeader(llc);
    Ipv4Header ipv4Header;
    pCopy->RemoveHeader (ipv4Header);
    Ipv4Address destinIpv4Addr = ipv4Header.GetDestination ();

    std::map<Ipv4Address, uint64_t>::iterator itBytesCounter =  m_bytesEnqueue.find(destinIpv4Addr);
    if (itBytesCounter !=  m_bytesEnqueue.end())
    {
        itBytesCounter->second += packet->GetSize();// + hdr.GetSize() + WIFI_MAC_FCS_LENGTH;
    }
    else
    {
        NS_LOG_WARN(" Packet enqueued for un-monitored flow");        
    }        


    if (m_stationManager->IsBrandNew (to))
    {
	// In ad hoc mode, we assume that every destination supports all
	// the rates we support.
	m_stationManager->AddAllSupportedModes (to);
	m_stationManager->RecordDisassociated (to);
    }

    WifiMacHeader hdr;

    // If we are not a QoS STA then we definitely want to use AC_BE to
    // transmit the packet. A TID of zero will map to AC_BE (through \c
    // QosUtilsMapTidToAc()), so we use that as our default here.
    uint8_t tid = 0;

    // For now, a STA that supports QoS does not support non-QoS
    // associations, and vice versa. In future the STA model should fall
    // back to non-QoS if talking to a peer that is also non-QoS. At
    // that point there will need to be per-station QoS state maintained
    // by the association state machine, and consulted here.
    if (m_qosSupported)
    {
      hdr.SetType(WIFI_MAC_QOSDATA);
      hdr.SetQosAckPolicy(WifiMacHeader::NORMAL_ACK);
      hdr.SetQosNoEosp ();
      hdr.SetQosNoAmsdu ();
      // Transmission of multiple frames in the same TXOP is not
      // supported for now
      hdr.SetQosTxopLimit (0);

      // Fill in the QoS control field in the MAC header
      tid = QosUtilsGetTidForPacket (packet);
      // Any value greater than 7 is invalid and likely indicates that
      // the packet had no QoS tag, so we revert to zero, which'll
      // mean that AC_BE is used.
      if (tid >= 7)
        {
          tid = 0;
        }
      hdr.SetQosTid (tid);
    }
    else
    {
	hdr.SetTypeData ();
    }

    hdr.SetAddr1(to);
    hdr.SetAddr2(m_low->GetAddress());
    hdr.SetAddr3(GetBssid());
    hdr.SetDsNotFrom();
    hdr.SetDsNotTo();

    if (m_qosSupported)
    {
	// Sanity check that the TID is valid
	NS_ASSERT (tid < 8);
	m_edca[QosUtilsMapTidToAc (tid)]->Queue(packet, hdr);
    }
    /*
    else
    {
	m_dca->Queue (packet, hdr);
    }
    */
}

void DmgWifiMac::SetLinkUpCallback(Callback<void> linkUp)
{
    NS_LOG_FUNCTION (this);
    m_linkUp = linkUp;
    linkUp();
}

void DmgWifiMac::SetLinkDownCallback(Callback<void> linkDown)
{
    NS_LOG_FUNCTION (this);
    m_linkDown = linkDown;
}

void DmgWifiMac::SetQosSupported(bool enable)
{
    NS_LOG_FUNCTION (this);
    m_qosSupported = enable;
}

bool DmgWifiMac::GetQosSupported() const
{
    return m_qosSupported;
}

void DmgWifiMac::SetHtSupported(bool enable)
{
    NS_LOG_FUNCTION(this);
    m_htSupported = enable;
}

bool DmgWifiMac::GetHtSupported() const
{
    return m_htSupported;
}

void DmgWifiMac::SetCtsToSelfSupported(bool enable)
{
    NS_LOG_FUNCTION (this);
    m_low->SetCtsToSelfSupported(enable);
}

bool DmgWifiMac::GetCtsToSelfSupported() const
{
    return m_low->GetCtsToSelfSupported();
}

void DmgWifiMac::SetSlot(Time slotTime)
{
    NS_LOG_FUNCTION(this << slotTime);
    m_dcfManager->SetSlot(slotTime);
    m_low->SetSlotTime(slotTime);
}

Time DmgWifiMac::GetSlot(void) const
{
    return m_low->GetSlotTime();
}

void DmgWifiMac::SetSifs (Time sifs)
{
    NS_LOG_FUNCTION (this << sifs);
    m_dcfManager->SetSifs(sifs);
    m_low->SetSifs(sifs);
}

Time DmgWifiMac::GetSifs (void) const
{
    return m_low->GetSifs();
}

void DmgWifiMac::SetEifsNoDifs(Time eifsNoDifs)
{
    NS_LOG_FUNCTION(this << eifsNoDifs);
    m_dcfManager->SetEifsNoDifs(eifsNoDifs);
}

Time DmgWifiMac::GetEifsNoDifs(void) const
{
    return m_dcfManager->GetEifsNoDifs();
}

void DmgWifiMac::SetRifs(Time rifs)
{
    NS_LOG_FUNCTION(this << rifs);
    m_low->SetRifs(rifs);
}

Time DmgWifiMac::GetRifs(void) const
{
    return m_low->GetRifs();
}

void DmgWifiMac::SetPifs(Time pifs)
{
    NS_LOG_FUNCTION (this << pifs);
    m_low->SetPifs(pifs);
}

Time DmgWifiMac::GetPifs(void) const
{
    return m_low->GetPifs();
}

void DmgWifiMac::SetAckTimeout(Time ackTimeout)
{
    NS_LOG_FUNCTION (this << ackTimeout);
    m_low->SetAckTimeout (ackTimeout);
}

Time DmgWifiMac::GetAckTimeout(void) const
{
    return m_low->GetAckTimeout();
}

void DmgWifiMac::SetCtsTimeout(Time ctsTimeout)
{
    NS_LOG_FUNCTION(this << ctsTimeout);
    m_low->SetCtsTimeout(ctsTimeout);
}

Time DmgWifiMac::GetCtsTimeout(void) const
{
    return m_low->GetCtsTimeout();
}

void DmgWifiMac::SetBasicBlockAckTimeout (Time blockAckTimeout)
{
    NS_LOG_FUNCTION (this << blockAckTimeout);
    m_low->SetBasicBlockAckTimeout (blockAckTimeout);
}

Time DmgWifiMac::GetBasicBlockAckTimeout(void) const
{
    return m_low->GetBasicBlockAckTimeout();
}

void DmgWifiMac::SetCompressedBlockAckTimeout (Time blockAckTimeout)
{
    NS_LOG_FUNCTION(this << blockAckTimeout);
    m_low->SetCompressedBlockAckTimeout(blockAckTimeout);
}

Time DmgWifiMac::GetCompressedBlockAckTimeout (void) const
{
    return m_low->GetCompressedBlockAckTimeout();
}

void DmgWifiMac::SetAddress(Mac48Address address)
{
    NS_LOG_FUNCTION(this << address);
    m_low->SetAddress(address);
    m_low->SetBssid(address);
    m_self = address;
}

Mac48Address DmgWifiMac::GetAddress(void) const
{
    return m_low->GetAddress();
}

void DmgWifiMac::SetSsid(Ssid ssid)
{
    NS_LOG_FUNCTION (this << ssid);
    m_ssid = ssid;
}

Ssid DmgWifiMac::GetSsid (void) const
{
    return m_ssid;
}

void DmgWifiMac::SetBssid(Mac48Address bssid)
{
    NS_LOG_FUNCTION(this << bssid);
    m_low->SetBssid(bssid);
}

Mac48Address DmgWifiMac::GetBssid(void) const
{
    return m_low->GetBssid();
}

void DmgWifiMac::SetPromisc(void)
{
    m_low->SetPromisc();
}

void DmgWifiMac::Enqueue(Ptr<const Packet> packet, Mac48Address to, Mac48Address from)
{
  // We expect DmgWifiMac subclasses which do support forwarding (e.g.,
  // AP) to override this method. Therefore, we throw a fatal error if
  // someone tries to invoke this method on a class which has not done
  // this.
  NS_FATAL_ERROR("This MAC entity (" << this << ", " << GetAddress ()
				     << ") does not support Enqueue() with from address");
}

bool DmgWifiMac::SupportsSendFrom(void) const
{
    return false;
}

void DmgWifiMac::ForwardUp(Ptr<Packet> packet, Mac48Address from, Mac48Address to)
{
    NS_LOG_FUNCTION(this << packet << from);
    m_forwardUp(packet, from, to);
}

void DmgWifiMac::Receive(Ptr<Packet> packet, const WifiMacHeader *hdr)
{
    NS_LOG_FUNCTION(this << packet << hdr);
    NS_ASSERT (!hdr->IsCtl ());
    Mac48Address to = hdr->GetAddr1();
    Mac48Address from = hdr->GetAddr2();

    // Rx rate measurement
    Ptr<Packet> pCopy = packet->Copy ();
    NS_LOG_DEBUG (Simulator::Now ()<< " rx size " << packet->GetSize());// + hdr->GetSize()+ WIFI_MAC_FCS_LENGTH); //MPDU tags (4bytes not inclueded)

  if (hdr->IsQosData ())
  {
          LlcSnapHeader llc;
          pCopy->RemoveHeader(llc);
          Ipv4Header ipv4Header;
          pCopy->RemoveHeader (ipv4Header);
          Ipv4Address srcIpv4Addr = ipv4Header.GetSource ();

          std::map<Ipv4Address, uint64_t>::iterator itBytesCounter =  m_bytesRx.find(srcIpv4Addr);
          if (itBytesCounter !=  m_bytesRx.end())
          {
                itBytesCounter->second += packet->GetSize();// + hdr->GetSize() + WIFI_MAC_FCS_LENGTH;
          }
          else
          {
                NS_LOG_DEBUG("packet rx-ed from un-monitored flow");      
                m_bytesRx.insert(std::pair<Ipv4Address, uint64_t>(srcIpv4Addr, packet->GetSize()));// + hdr->GetSize()+ WIFI_MAC_FCS_LENGTH));  
          }        
  }

    // We don't know how to deal with any frame that is not addressed to
    // us (and odds are there is nothing sensible we could do anyway),
    // so we ignore such frames.
    //
    // The derived class may also do some such filtering, but it doesn't
    // hurt to have it here too as a backstop.
    if (to != GetAddress())
    {
	return;
    }

    if (hdr->IsData ())
    {
      if (hdr->IsQosData () && hdr->IsQosAmsdu ())
        {
          NS_LOG_DEBUG ("Received A-MSDU from" << from);
          DeaggregateAmsduAndForward (packet, hdr);
        }
      else
        {
          ForwardUp (packet, from, to);
        }
      return;
    }

    if (hdr->IsMgt() && hdr->IsAction()) {
	// There is currently only any reason for Management Action
	// frames to be flying about if we are a QoS STA.
	NS_ASSERT(m_qosSupported);

	WifiActionHeader actionHdr;
	packet->RemoveHeader (actionHdr);

	switch (actionHdr.GetCategory ())
        {
	    case WifiActionHeader::BLOCK_ACK:
	    {
		switch (actionHdr.GetAction ().blockAck)
		{
		    case WifiActionHeader::BLOCK_ACK_ADDBA_REQUEST:
		    {
			MgtAddBaRequestHeader reqHdr;
			packet->RemoveHeader (reqHdr);

			// We've received an ADDBA Request. Our policy here is
			// to automatically accept it, so we get the ADDBA
			// Response on it's way immediately.
			SendAddBaResponse (&reqHdr, from);

			// This frame is now completely dealt with, so we're done.
			return;
		    }

		    case WifiActionHeader::BLOCK_ACK_ADDBA_RESPONSE:
		    {
			MgtAddBaResponseHeader respHdr;
			packet->RemoveHeader (respHdr);

			// We've received an ADDBA Response. We assume that it
			// indicates success after an ADDBA Request we have
			// sent (we could, in principle, check this, but it
			// seems a waste given the level of the current model)
			// and act by locally establishing the agreement on
			// the appropriate queue.
			AcIndex ac = QosUtilsMapTidToAc (respHdr.GetTid ());
			m_edca[ac]->GotAddBaResponse (&respHdr, from);

			// This frame is now completely dealt with, so we're done.
			return;
		    }

		    case WifiActionHeader::BLOCK_ACK_DELBA:
		    {
			MgtDelBaHeader delBaHdr;
			packet->RemoveHeader(delBaHdr);

			if (delBaHdr.IsByOriginator ())
			{
			    // This DELBA frame was sent by the originator, so
			    // this means that an ingoing established
			    // agreement exists in MacLow and we need to
			    // destroy it.
			    m_low->DestroyBlockAckAgreement (from, delBaHdr.GetTid ());
			}
			else
			{
			    // We must have been the originator. We need to
			    // tell the correct queue that the agreement has
			    // been torn down
			    AcIndex ac = QosUtilsMapTidToAc (delBaHdr.GetTid ());
			    m_edca[ac]->GotDelBaFrame (&delBaHdr, from);
			}
			// This frame is now completely dealt with, so we're done.
			return;
		    }

		    default:
			NS_FATAL_ERROR ("Unsupported Action field in Block Ack Action frame");
			return;
		}
            }

	    default:
		NS_FATAL_ERROR ("Unsupported Action frame received");
		return;

        }
    }
    NS_FATAL_ERROR("Don't know how to handle frame (type=" << hdr->GetType ());
}

void DmgWifiMac::DeaggregateAmsduAndForward(Ptr<Packet> aggregatedPacket, const WifiMacHeader *hdr)
{
    MsduAggregator::DeaggregatedMsdus packets = MsduAggregator::Deaggregate (aggregatedPacket);

    for (MsduAggregator::DeaggregatedMsdusCI i = packets.begin (); i != packets.end (); ++i)
    {
	ForwardUp((*i).first, (*i).second.GetSourceAddr (),
		  (*i).second.GetDestinationAddr ());
    }
}

void DmgWifiMac::SendAddBaResponse(const MgtAddBaRequestHeader *reqHdr, Mac48Address originator)
{
    NS_LOG_FUNCTION (this);
    WifiMacHeader hdr;
    hdr.SetAction ();
    hdr.SetAddr1 (originator);
    hdr.SetAddr2 (GetAddress ());
    hdr.SetAddr3 (GetAddress ());
    hdr.SetDsNotFrom ();
    hdr.SetDsNotTo ();

    MgtAddBaResponseHeader respHdr;
    StatusCode code;
    code.SetSuccess ();
    respHdr.SetStatusCode (code);
    //Here a control about queues type?
    respHdr.SetAmsduSupport (reqHdr->IsAmsduSupported ());

    if (reqHdr->IsImmediateBlockAck ())
    {
	respHdr.SetImmediateBlockAck ();
    }
    else
    {
	respHdr.SetDelayedBlockAck ();
    }
    respHdr.SetTid (reqHdr->GetTid ());
    // For now there's not no control about limit of reception. We
    // assume that receiver has no limit on reception. However we assume
    // that a receiver sets a bufferSize in order to satisfy next
    // equation: (bufferSize + 1) % 16 = 0 So if a recipient is able to
    // buffer a packet, it should be also able to buffer all possible
    // packet's fragments. See section 7.3.1.14 in IEEE802.11e for more
    // details.
    respHdr.SetBufferSize (1023);
    respHdr.SetTimeout (reqHdr->GetTimeout ());

    WifiActionHeader actionHdr;
    WifiActionHeader::ActionValue action;
    action.blockAck = WifiActionHeader::BLOCK_ACK_ADDBA_RESPONSE;
    actionHdr.SetAction (WifiActionHeader::BLOCK_ACK, action);

    Ptr<Packet> packet = Create<Packet> ();
    packet->AddHeader (respHdr);
    packet->AddHeader (actionHdr);

    // We need to notify our MacLow object as it will have to buffer all
    // correctly received packets for this Block Ack session
    m_low->CreateBlockAckAgreement(&respHdr, originator, reqHdr->GetStartingSequence ());

    // It is unclear which queue this frame should go into. For now we
    // bung it into the queue corresponding to the TID for which we are
    // establishing an agreement, and push it to the head.
    m_edca[QosUtilsMapTidToAc (reqHdr->GetTid ())]->PushFront (packet, hdr);
}

void DmgWifiMac::FinishConfigureStandard(enum WifiPhyStandard standard)
{
    uint32_t cwmin;
    uint32_t cwmax;

    switch (standard)
    {
	case WIFI_PHY_STANDARD_80211ad:
	{
	    cwmin = 15;
	    cwmax = 1023;
	    break;
	}
	default:
	  NS_FATAL_ERROR ("Unsupported WifiPhyStandard in DmgWifiMac::FinishConfigureStandard ()");

    }

    // The special value of AC_BE_NQOS which exists in the Access
    // Category enumeration allows us to configure plain old DCF.
    //ConfigureDcf(m_dca, cwmin, cwmax, AC_BE_NQOS);

    // Now we configure the EDCA functions
    for (EdcaQueues::iterator i = m_edca.begin (); i != m_edca.end (); ++i)
    {
	ConfigureDcf (i->second, cwmin, cwmax, i->first);
    }
}

void DmgWifiMac::TxOk(const WifiMacHeader &hdr)
{
    NS_LOG_FUNCTION(this << hdr);
    m_txOkCallback(hdr);
}

void DmgWifiMac::TxFailed(const WifiMacHeader &hdr)
{
    NS_LOG_FUNCTION(this << hdr);
    m_txErrCallback(hdr);
}

void
DmgWifiMac::ConfigureCw (uint32_t cwmin, uint32_t cwmax)
{
  //The special value of AC_BE_NQOS which exists in the Access
  //Category enumeration allows us to configure plain old DCF.
  //ConfigureDcf (m_dca, cwmin, cwmax, AC_BE_NQOS);

  //Now we configure the EDCA functions
  for (EdcaQueues::iterator i = m_edca.begin (); i != m_edca.end (); ++i)
    {
      ConfigureDcf (i->second, cwmin, cwmax, i->first);
    }

}

void
DmgWifiMac::SetDmgAntennaController (Ptr<DmgAntennaController> dmgAntennaController)
{
  NS_ASSERT(m_phy);
  m_dmgAntennaController = dmgAntennaController;
  m_phy->GetObject<YansWifiPhy>()->SetDmgAntennaController(dmgAntennaController);
  m_phy->GetObject<YansWifiPhy>()->SetAddress(m_self);
  for (EdcaQueues::iterator i = m_edca.begin(); i != m_edca.end(); ++i)
  {
    i->second->SetDmgAntennaController(dmgAntennaController);
  }
}

Ptr<DmgAntennaController>
DmgWifiMac::GetDmgAntennaController (void)
{
  return m_dmgAntennaController;
}

void
DmgWifiMac::SetDmgBeaconInterval(Ptr<DmgBeaconInterval> dmgBi)
{
  NS_ASSERT(m_dmgAntennaController);
  m_dmgBeaconInterval = dmgBi;
  m_dmgBeaconInterval->SetDmgAntennaController(m_dmgAntennaController);
  m_low->SetDmgBeaconInterval(dmgBi);
  m_dcfManager->SetDmgBeaconInterval(dmgBi);
  for (EdcaQueues::iterator i = m_edca.begin(); i != m_edca.end(); ++i)
  {
    i->second->SetDmgBeaconInterval(dmgBi);
  }
}

void
DmgWifiMac::StartDmgSpTracking (void)
{
  return;
    
  for (EdcaQueues::iterator i = m_edca.begin(); i != m_edca.end(); ++i)
  {
    i->second->StartDmgSpTracking();
  }
}

Ptr<DmgBeaconInterval>
DmgWifiMac::GetDmgBeaconInterval(void)
{
  return m_dmgBeaconInterval;
}

void
DmgWifiMac::SetPropagationGuard(Time guard)
{
  m_dcfManager->SetPropagationGuard(guard);
  m_low->SetPropagationGuard(guard);
}

void
DmgWifiMac::SetDmgAckTimeoutGuard (Time guard)
{
  m_dmgAckTimeoutGuard = guard;
}

Time
DmgWifiMac::GetDmgAckTimeoutGuard (void) const
{
  return m_dmgAckTimeoutGuard;
}

uint64_t 
DmgWifiMac::GetBytesRxFrom (Ipv4Address src)
{
  NS_LOG_FUNCTION (this);

  std::map<Ipv4Address, uint64_t>::iterator itBytesCounter =  m_bytesRx.find(src);

  uint64_t bytes = 0;
  if (itBytesCounter !=  m_bytesRx.end())
        bytes = itBytesCounter->second;
  NS_LOG_INFO (GetAddress() << " total Rx from src " << src <<" bytes "<< bytes);

  return bytes;
}

void 
DmgWifiMac::InstallEnqueueRateMeter (Ipv4Address destin)
{
  NS_LOG_FUNCTION (this);
  m_startEnqueueMeasureT.insert(std::pair<Ipv4Address, Time>(destin, Simulator::Now()));
  m_bytesEnqueue.insert(std::pair<Ipv4Address, uint64_t>(destin, 0));
}


void 
DmgWifiMac::ResetEnqueueRateMeter (Ipv4Address destin)
{
  NS_LOG_FUNCTION (this);
  std::map<Ipv4Address, Time>::iterator itStartTime =  m_startEnqueueMeasureT.find(destin);
  NS_ASSERT (itStartTime !=  m_startEnqueueMeasureT.end());

  itStartTime->second = Simulator::Now();

  std::map<Ipv4Address, uint64_t>::iterator itBytesCounter =  m_bytesEnqueue.find(destin);
  NS_ASSERT (itBytesCounter !=  m_bytesEnqueue.end());

  itBytesCounter->second = 0;
}


uint32_t 
DmgWifiMac::CalcEnqueueRate (Ipv4Address destin)
{
  NS_LOG_FUNCTION (this);

  std::map<Ipv4Address, Time>::iterator itStartTime =  m_startEnqueueMeasureT.find(destin);
  std::map<Ipv4Address, uint64_t>::iterator itBytesCounter =  m_bytesEnqueue.find(destin);
  NS_ASSERT (itStartTime !=  m_startEnqueueMeasureT.end());
  NS_ASSERT (itBytesCounter !=  m_bytesEnqueue.end());
	
  uint32_t rate = itBytesCounter->second * 8.0 / (Simulator::Now().GetMicroSeconds() - itStartTime->second.GetMicroSeconds());
  NS_LOG_INFO (" Destin " << destin <<" rate "<< rate <<" Mb/s");

  return rate;
}

Time
DmgWifiMac::GetNMpduReturnDuration(Mac48Address mac)
{
  NS_LOG_FUNCTION (this);

  Time duration = m_low->GetNMpduReturnDuration(mac);

  return duration;
}

void 
DmgWifiMac::SetMaxNumMpdu(uint32_t num)
{
        m_low->SetMaxNumMpdu(num);
}
} // namespace ns3
