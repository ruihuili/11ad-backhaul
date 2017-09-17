#include <iomanip>
#include <fstream>
#include <cstdlib>
#include <sstream>
#include "ns3/log.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/wifi-module.h"
#include "ns3/wifi-mac-queue.h"
#include "ns3/mobility-module.h"
#include "ns3/internet-module.h"

using namespace ns3;

#define SSID_STR		"test"
#define DESTINATION_PORT_BASE	20000

NS_LOG_COMPONENT_DEFINE ("DmgSimple");

struct sim_config {
	/* Number of mesh nodes */
	uint32_t meshNodesN;
	/* Mesh nodes cointainer */
	NodeContainer *meshNodes;

	/* Pointer to the Oracle controller (DmgAlmightyController)
	 * The controller knows every node in the network
	 */
	Ptr<DmgAlmightyController> dmgCtrl;

	/* Simulation duration 
	 * Nodes transmit data from second 1.0 to second simuationTime + 1.0
	 */
	double simulationTime;

	int16_t scenario;

	/* UDP payload bytes */
	uint32_t appPayloadBytes;

	/* if true pcap capture files will be generated in the working directory */
	bool enablePcap;

	/* All the nodes will be placed randomly in a square
	 * square_side_l x square_side_l (meters)
	 */
	double square_side_l;

	/* List of UDP servers.
	 * Used for reporting the througput
	 */
	ApplicationContainer *servers;
	/* List of UDP clients.
	 * Used for reconfiguring flow demands
	 */
	ApplicationContainer *clients;

	/* support state variable for throughput report generation */
	Time *lastReportTime;

	std::vector <double> lastRxMbits;
	std::vector <double> lastTxMbits;
	std::vector <double> lastMacRxMbits;

	/* Minimum frame Bit Error Rate (BER).
	 * Remember that a complete frame exchange is composed by a data
	 * frame + ack frame. The minimum BER is applied to both the frames. If for a
	 * given station you want a Frame Loss Rate (FLR) l then:
	 * frameMinBer  = 1 - sqrt(1 - l, bits_ack + bits_data)
	 */
	double frameMinBer;

	/* Antenna gain in dBi.
	 * This value is the same for all the nodes and is applied both
	 * during reception and during transmission
	 */
	double txRxAntennaGainDbi;

	/* Reception and transmission gain in dBi.
	 * The gain is the same for all the nodes
	 */
	double txRxGainDbi;

	/* Transmission power in dBi.
	 * All the nodes use the same transmission power.
	 */ 
	double txPowerDbi;

	/* Beacon Interval (BI) duration in ns */
	uint64_t biDurationNs;

	/* Fraction of the BI reserved for protocol overhead.
	 * This is the same for all the APs
	 */
	double biOverheadFraction;

	/* true for using DMG OFDM PHY.
	 * false for using SC OFDM PHY.
	 * MCS0 (CTRL) is always enabled
	 */
	bool dmgOfdm;

	/* Max number of aggregated MPDU */
	uint32_t nMpdus;  

	/* Time in Nano seconds that represent the time consumed
	 * when switch beam to different destination*/
	uint64_t beamSwitchOverhead;

	/* Time in Nano seconds, after which the pregressive filling will 
	 * re-run to adapt to new Udp traffic demands*/
	uint64_t refillInterval;

	/* Traffic type used for simulation
	 * for now the options are udp and onoff*/
	std::string trafficType;

	std::vector < std::vector <float> > nodesPosition;
	std::vector < std::vector <uint32_t> > flowsPath;

	std::vector < double > flowsDmd;
	std::vector < double > predictedFlowRate;

	//Maximum queue size in wifi mac queue
	uint32_t macQueueSizeinPkts;
	std::string inputFileName;

	std::vector < uint32_t > m_nextHop;
	double proFillStepL;
	bool ifPrintThrpt;
	std::vector <std::vector <uint64_t> > lastTxSuccess;
	std::vector <std::vector <uint64_t> > lastTxFailure;
	uint64_t lastRx;

	Ptr<ExponentialRandomVariable> rng;
	uint32_t numSchedulePerBi;
	double ackTraffFrac;

	std::ostringstream dir_oss;
	std::vector < Ptr<OutputStreamWrapper> > streams_tp;

};

struct sim_config config;

/* Utility function used to print the simulation configuration */
void printSimInfo ()
{
	NS_LOG_INFO( "*********************************" );
	NS_LOG_INFO( "NODES POSITIONS (x, y)" );
	NS_LOG_INFO( "*********************************");

	for (uint32_t i = 0; i < config.meshNodesN; i++) {
		Vector pos =
			config.meshNodes->Get(i)->GetObject<MobilityModel>()->GetPosition();
		NS_LOG_INFO("Mesh" << i << " (" << pos.x << ", " << pos.y << ")");
	}


	NS_LOG_INFO("*********************************");
	NS_LOG_INFO("SIMULATION CONFIGURATION");
	NS_LOG_INFO("*********************************");

	NS_LOG_INFO("Simulation time in seconds: " << config.simulationTime);

	NS_LOG_INFO("11ad mode: " << (config.dmgOfdm ? "OFDM" : "SC") );

	NS_LOG_INFO("UDP payload " << config.appPayloadBytes << " bytes");

	NS_LOG_INFO("PCAP capture enabled: " << (config.enablePcap ? "true" : "false"));

	NS_LOG_INFO("Min Frame BER: " << config.frameMinBer);

	NS_LOG_INFO("Transmission power: " << config.txPowerDbi << "dBi" );

	NS_LOG_INFO("Transmission and Reception gain: " << config.txRxGainDbi << "dBi" );

	NS_LOG_INFO("TX and RX antenna gain: " << config.txRxAntennaGainDbi << "dBi");

	NS_LOG_INFO("Antenna beamwidth degrees: " <<
			config.meshNodes->Get(0)->GetDevice(0)->GetObject<WifiNetDevice>()->
			GetMac()->GetObject<DmgWifiMac>()->GetDmgAntennaController()->
			GetBeamwidthDegrees() );

	NS_LOG_INFO("BI duration: " << config.biDurationNs << "ns" );

	NS_LOG_INFO("BI overhead: " << (uint64_t)(config.biOverheadFraction *
				config.biDurationNs) << "ns" );

	NS_LOG_INFO("Max number of packets stored in Mac queue: " << config.macQueueSizeinPkts);

	NS_LOG_INFO("Topology read from file: " << config.inputFileName);

	NS_LOG_INFO( "Progressive filling step length: " << config.proFillStepL << "Mb/s");

	NS_LOG_INFO( "Print throughput info in to a file: (Yes if 1)" << config.ifPrintThrpt);

	NS_LOG_INFO( "Beam switching overhead: " << config.beamSwitchOverhead << "ns" );

	NS_LOG_INFO( "Re-fill interval: " << config.refillInterval << "ns");

	NS_LOG_INFO( "Traffic Type: " << config. trafficType );

	if (config.nMpdus > 0) {
		NS_LOG_INFO( "Aggregation enabled: A-MPDU max size -> " << config.nMpdus <<
				" MPDUs" );
	} else {
		NS_LOG_INFO( "Aggregation disabled");
	}

	NS_LOG_INFO("Simulating " << config.meshNodesN << " mesh stations.");

	NS_LOG_INFO("*********************************");
	NS_LOG_INFO("IDEAL RX POWER and MCS");
	NS_LOG_INFO("*********************************");
	config.dmgCtrl->PrintIdealRxPowerAndMcs();

	config.dmgCtrl->PrintSpInfo();

	NS_LOG_INFO( "Number of schedule per beacon interval: " << config.numSchedulePerBi);

	NS_LOG_INFO( "Fraction of time used for traffic of inverse direction (e.g. tcp ack) " << config.ackTraffFrac);
}

void CreateTpFiles(struct sim_config *config)
{

	Time now_t = Simulator::Now ();

	/* The first time the function is called we schedule the next call after
	 * biDurationNs ns and then return
	 */

	NS_LOG_INFO(now_t);
	*(config->lastReportTime) = now_t;

	for(uint32_t flowIdx = 0; flowIdx < config->flowsPath.size(); flowIdx++){
		std::vector <uint64_t> nRxer (config->flowsPath.at(flowIdx).size() - 1, 0);
		config->lastTxSuccess.push_back(nRxer);
		config->lastTxFailure.push_back(nRxer);
		config->lastRxMbits.push_back(0);
		config->lastTxMbits.push_back(0);
		config->lastMacRxMbits.push_back(0);

		std::ostringstream fileName_oss;

		std::string stringdir = config->dir_oss.str ();
		fileName_oss << stringdir << "tp-"  << flowIdx ;


		AsciiTraceHelper ascii;
		Ptr<OutputStreamWrapper> stream = ascii.CreateFileStream (fileName_oss.str ());

		*stream->GetStream() <<"# measuredRate\tpredictedRete\trxRate\ttxRate"<< std::endl;
		config->streams_tp.push_back(stream);

	}
	*(config->lastReportTime) = now_t;
	NS_LOG_INFO(now_t);
	return;
}

/* Utility function used to generate the throughput (+ successes and failures)
 * report. This function is called every biDurationNs ns.
 */
void WriteTpFiles(struct sim_config *config)
{
	Time now_t = Simulator::Now ();
	for(uint32_t flowIdx = 0; flowIdx < config->flowsPath.size(); flowIdx++){

		/* print the simulation time in the first column of the report file */
		*config->streams_tp.at(flowIdx)->GetStream() << now_t.GetSeconds () << " ";
		*config->streams_tp.at(flowIdx)->GetStream() << std::fixed << std::setprecision(6);

		uint64_t rx_packets;

		Ptr<EdcaTxopN> edca_src = config->meshNodes->Get(config->flowsPath.at(flowIdx).front())->GetDevice(0)->GetObject<WifiNetDevice>()->GetMac()->GetObject<DmgWifiMac> ()->GetBEQueue ();


		uint64_t rx_bytes = 0;//SUM of subflows
		uint64_t tx_bytes = 0;//SUM of subflows

			rx_bytes = DynamicCast<PacketSink>(config->servers->Get (flowIdx))->GetTotalRx ();//the total bytes received in this sink
			tx_bytes = DynamicCast<MyOnOffApplication>(config->clients->Get (flowIdx))->GetTotalTxBytes ();

		double cur_rx_Mbits = (rx_bytes * 8.0) / 1e6;
		double cur_tx_Mbits = (tx_bytes * 8.0) / 1e6;


		//mac throughput
		Ipv4Address srcIp = config->meshNodes->Get(config->flowsPath.at(flowIdx).front())->GetObject<Ipv4>()->GetAddress(1,0).GetLocal();
		uint64_t rx_bytes_mac = config->meshNodes->Get(config->flowsPath.at(flowIdx).back())->GetDevice(0)->GetObject<WifiNetDevice>()->GetMac()->GetObject<DmgWifiMac>()->GetBytesRxFrom(srcIp);

		double cur_rx_Mbits_mac = (rx_bytes_mac * 8.0) / 1e6;

		NS_LOG_UNCOND(now_t << " flow " << flowIdx << "\t"<< " rx rate: " << (cur_rx_Mbits - config->lastRxMbits[flowIdx]) / ((now_t - *(config->lastReportTime)).GetNanoSeconds() / 1e9) <<" Mbps.");

		*config->streams_tp.at(flowIdx)->GetStream() << (cur_rx_Mbits - config->lastRxMbits[flowIdx]) / ((now_t - *(config->lastReportTime)).GetNanoSeconds() / 1e9) << std::endl;

		config->lastRxMbits[flowIdx] = cur_rx_Mbits;
		config->lastTxMbits[flowIdx] = cur_tx_Mbits;
		config->lastMacRxMbits[flowIdx] = cur_rx_Mbits_mac;
	}
	config->dmgCtrl->PrintIdealRxPowerAndMcs();

	Simulator::Schedule(NanoSeconds(config->biDurationNs), WriteTpFiles, config);
	*(config->lastReportTime) = now_t;
}

void PrintQueueSize(struct sim_config *config,
		Ptr<OutputStreamWrapper> stream_queuesize)
{
	*stream_queuesize->GetStream() << Simulator::Now ().GetSeconds();
	//for each station
	for(uint32_t staIdx=0; staIdx<config->meshNodes->GetN(); staIdx++ )
	{
		*stream_queuesize->GetStream() << "\t" << config->meshNodes->Get(staIdx)->GetDevice(0)->GetObject<WifiNetDevice>()->GetMac()->GetObject<DmgWifiMac> ()->GetBEQueue () -> GetEdcaQueue() ->GetSize();
	}

	//for each flow (destination)
	for (uint32_t flowIdx = 0; flowIdx< config->flowsPath.size(); flowIdx++)
	{
		for (uint32_t i = 0; i < config->flowsPath.at(flowIdx).size()-1; i++) 
		{
			uint32_t staId = config->flowsPath.at(flowIdx).at(i);
			uint32_t nexthopId = config->flowsPath.at(flowIdx).at(i+1);
			uint32_t srcId = config->flowsPath.at(flowIdx).front();
			uint32_t destId = config->flowsPath.at(flowIdx).back();

			Mac48Address nexthopMacAddr = config->meshNodes->Get(nexthopId)->GetDevice(0)->GetObject<WifiNetDevice>()->GetMac()->GetObject<DmgWifiMac> ()->GetAddress();

			std::pair <Ipv4Address, Ipv4Address> srcsinkAddr;
			srcsinkAddr.first = config->meshNodes->Get(srcId)->GetObject<Ipv4>()->GetAddress(1,0).GetLocal();
			srcsinkAddr.second = config->meshNodes->Get(destId)->GetObject<Ipv4>()->GetAddress(1,0).GetLocal();

			*stream_queuesize->GetStream() << "\t"<< staId <<"to" << nexthopId <<"["<< destId <<"]\t" << config->meshNodes->Get(staId)->GetDevice(0)->GetObject<WifiNetDevice>()->GetMac()->GetObject<DmgWifiMac> ()->GetBEQueue () -> GetEdcaQueue()->GetNPacketsByAddress(WifiMacHeader::ADDR1,nexthopMacAddr, srcsinkAddr);
		}
	}
	*stream_queuesize->GetStream() << std::endl;
	Simulator::Schedule(NanoSeconds(10000), PrintQueueSize, config, stream_queuesize);
}

/* Parse the Inet format topology input file*/
	void
ParsingTopologyFromFile(struct sim_config *config)
{
	std::ifstream topgen;
	topgen.open(config->inputFileName.c_str());
	if ( !topgen.is_open () )
	{
		NS_LOG_INFO ("file object is not open, check file name and permissions");
		return;
	}

	std::string from;
	std::string to;
	std::string nodeIdx;
	std::string yCoordinate;
	std::string xCoordinate;
	std::string flowHop;
	std::string flowDmd;

	uint32_t totflow = 0;

	std::istringstream lineBuffer;
	std::string line;

	getline (topgen,line);

	if (line.length() > 0 && line.find('#') != std::string::npos){
		getline (topgen,line);
	}

	lineBuffer.str (line);
	lineBuffer >> config->meshNodesN;
	lineBuffer >> totflow;

	//Read location info
	for (uint32_t i = 0; i < config->meshNodesN && !topgen.eof (); i++)
	{
		getline (topgen,line);
		lineBuffer.clear ();
		lineBuffer.str (line);

		lineBuffer >> nodeIdx;
		lineBuffer >> xCoordinate;
		lineBuffer >> yCoordinate;

		std::vector<float> location;
		location.push_back(std::atof(xCoordinate.c_str()));
		location.push_back(std::atof(yCoordinate.c_str()));

		config->nodesPosition.push_back(location);
	}
	//Read flow info
	config->flowsPath.resize(totflow);
	for ( uint32_t i = 0; i < totflow && !topgen.eof (); i++){
		getline (topgen,line);
		lineBuffer.clear ();
		lineBuffer.str (line);

		lineBuffer >> flowDmd;
		config->flowsDmd.push_back(std::atoi(flowDmd.c_str()));

		getline (topgen,line);
		lineBuffer.clear ();
		lineBuffer.str (line);

		uint32_t nline = line.size()/2;
		while(nline--){
			lineBuffer >> flowHop;
			if (flowHop != "#"){
				config->flowsPath.at(i).push_back(std::atoi(flowHop.c_str()));
			}
			else
				break;
		}
	}
}

/* Nodes configuration finalization:
 * - ARP setup
 * - Minimum frame BER setup
 */
	void
FinalizeConfig(struct sim_config *config)
{
	NodeContainer allNodes (*(config->meshNodes));

	for (uint32_t i = 0; i < allNodes.GetN(); i++)
	{
		/* This loop is used to force the ARP tables on each node */
		for (uint32_t j = 0; j < allNodes.GetN(); j++)
		{
			if (i == j)
				continue;

			Address mac = allNodes.Get(j)->GetDevice(0)->GetAddress();
			Ipv4Address ip = allNodes.Get(j)->GetObject<Ipv4>()->
				GetAddress(1,0).GetLocal();
			Ptr<ArpCache> arpCache = allNodes.Get(i)->GetObject<Ipv4L3Protocol>()->
				GetInterface(1)->GetArpCache();

			if (arpCache == NULL)
				arpCache = CreateObject<ArpCache>();
			arpCache->SetAliveTimeout(Seconds (config->simulationTime + 1));
			ArpCache::Entry *entry = arpCache->Add(ip);
			entry->MarkWaitReply(0);
			entry->MarkAlive(mac);
		}

		/* Configure the minimum frame BER */
		Ptr<YansWifiPhy> yansPhy = allNodes.Get(i)->GetDevice(0)->
			GetObject<WifiNetDevice>()->GetPhy()->
			GetObject<YansWifiPhy> ();
		yansPhy->SetFrameMinBer(config->frameMinBer);
	}
}

//Periodically called to cope with demand dynamics
void ReconfigureDmgBeaconInterval (struct sim_config *config, uint16_t it)
{
	Time now = Simulator::Now();
	Time bi = NanoSeconds(config->biDurationNs);
	Time overhead = NanoSeconds(ceil(config->biDurationNs * config->biOverheadFraction));
	Time nextBiStart = (now/bi + 1)* bi;

	NS_LOG_INFO("ReconfigureDmgBeaconInterval. Time now:"<< now );

	if ( now - floor(now / bi) * bi > overhead) {
		NS_LOG_INFO(now << "New schedule will be applied at: "<<nextBiStart );

		Simulator::Schedule(NanoSeconds(nextBiStart - now + 1), ReconfigureDmgBeaconInterval, config, 1);
		return;
	}

	/*To give an example of possible network dynamics scenario: additional signal loss between 0 and 3*/
	if (config->scenario == 5){
		config->dmgCtrl->EnforceAdditionalSignalLossBetween(config->meshNodes->Get(0), config->meshNodes->Get(3), 5 * it);
	}

	config->predictedFlowRate = config->dmgCtrl->FlowRateProgressiveFilling(config->flowsDmd, config->proFillStepL, config->appPayloadBytes, config->biOverheadFraction, config->nMpdus);

	config->dmgCtrl->ConfigureSchedule();

	config->dmgCtrl->ConfigureBeaconIntervals();

	if (config->nMpdus > 0) {
		/* Create the Block Ack Agreements */
		config->dmgCtrl->CreateBlockAckAgreement();
	}

	Simulator::Schedule(NanoSeconds(config->biDurationNs * 3 + 2), ReconfigureDmgBeaconInterval, config, it + 1);

	config->dmgCtrl->PrintSpInfo();

}

/* Utility function used to configure the DMG controller */
void SetupDmgController (struct sim_config *config)
{
	/* WARNING: in the current implementation it is suggested to call the
	 * functions used to configure the DmgALmightyController as in this example.
	 */
	/* We pass the container of the mesh nodes to the DmgAlmightyController */
	config->dmgCtrl->SetGw(0);
	config->dmgCtrl->SetMeshNodes(config->meshNodes);

	config->dmgCtrl->SetFlowsPath(config->flowsPath);

	config->dmgCtrl->ConfigureCliques();

	config->dmgCtrl->SetBeamSwitchOverhead(config->beamSwitchOverhead);

	/* Configure the DmgDestinationFixedWifiManager */
	config->dmgCtrl->ConfigureWifiManager();

	config->dmgCtrl->SetNumSchedulePerBi(config->numSchedulePerBi);

	config->dmgCtrl->SetAckTimeFrac(config->ackTraffFrac);

	config->predictedFlowRate = config->dmgCtrl->FlowRateProgressiveFilling(config->flowsDmd, config->proFillStepL, config->appPayloadBytes, config->biOverheadFraction, config->nMpdus);

	/* Let the DmgAlmightyController know the Beacon Interval duration */
	config->dmgCtrl->SetBiDuration(config->biDurationNs);
	/* Let the DmgAlmightyController know the fraction of the Beacon Interval that
	 * is considered overhead and is not used for data transmission */
	config->dmgCtrl->SetBiOverheadFraction(config->biOverheadFraction);

	/* Configure the DmgBeaconInterval on each node with the list of Secrive
	 * Periods.
	 */
	config->dmgCtrl->ConfigureSchedule();

	config->dmgCtrl->ConfigureBeaconIntervals();

	if (config->nMpdus > 0) {
		config->dmgCtrl->CreateBlockAckAgreement();
	}

	config->dmgCtrl->ConfigureWritePath(config->dir_oss.str());
}

/* Utility function used to finalize the configuration of the Nodes in the
 * network */
void SetupDmgNodes (struct sim_config *config)
{
	NodeContainer allNodes (*(config->meshNodes));//, *(config->ctrlNodes));

	for (uint32_t i = 0; i < allNodes.GetN(); i++) {
		/* Currently DMG nodes support only antennas whose type is ConeAntenna */
		Ptr<ConeAntenna> ant = CreateObject<ConeAntenna> ();
		/* Configure the gain (Rx and Tx) of the antenna */
		ant->SetGainDbi(config->txRxAntennaGainDbi);

		/* The antenna is managed by a DmgAntennaController */
		Ptr<DmgAntennaController> antCtrl = CreateObject<DmgAntennaController> ();
		antCtrl->SetAntenna(ant);
		antCtrl->SetPhy(allNodes.Get(i)->GetDevice(0)->GetObject<WifiNetDevice>()->
				GetPhy());

		/* Configure the DmgWifiMac with the DmgAntenna controller just created */
		allNodes.Get(i)->GetDevice(0)->GetObject<WifiNetDevice>()->
			GetMac()->GetObject<DmgWifiMac>()->SetDmgAntennaController(antCtrl);

		/* Configure the DmgWifiMac with a new DmgBeaconInterval */
		allNodes.Get(i)->GetDevice(0)->GetObject<WifiNetDevice>()->
			GetMac()->GetObject<DmgWifiMac>()->
			SetDmgBeaconInterval(CreateObject<DmgBeaconInterval>());

		/* It's important to configure a proper propagation time */
		allNodes.Get(i)->GetDevice(0)->GetObject<WifiNetDevice>()->
			GetMac()->GetObject<DmgWifiMac>()->SetPropagationGuard(
					Seconds(sqrt(pow(config->square_side_l, 2) * 2) /
						3e8));

		/* Configure number of mpdus to aggregated (used to measure return duration) */
		allNodes.Get(i)->GetDevice(0)->GetObject<WifiNetDevice>()->
			GetMac()->GetObject<DmgWifiMac>()->SetMaxNumMpdu(config->nMpdus);

		/* Configure phy level parameters */
		Ptr<YansWifiPhy> yPhy =
			allNodes.Get(i)->GetDevice(0)->GetObject<WifiNetDevice>()->
			GetPhy()->GetObject<YansWifiPhy>();
		yPhy->SetTxGain(config->txRxGainDbi);
		yPhy->SetRxGain(config->txRxGainDbi);
		yPhy->SetTxPowerStart(config->txPowerDbi);
		yPhy->SetTxPowerEnd(config->txPowerDbi);
		/* Sensitivity model includes implementation loss and noise figure */
		yPhy->SetRxNoiseFigure(0);
	}
}

void SetupOnOffTraffic(struct sim_config *config, std::string protocol)
{		
	for (uint32_t flowIdx = 0; flowIdx< config->flowsPath.size(); flowIdx++)
	{
		Ipv4Address ipServer = config->meshNodes->Get(config->flowsPath.at(flowIdx).back())->GetObject<Ipv4>()->GetAddress(1,0).GetLocal();
		AddressValue destinAddrValue (InetSocketAddress (ipServer, DESTINATION_PORT_BASE + flowIdx));     

		Ipv4Address ipClient = config->meshNodes->Get(config->flowsPath.at(flowIdx).front())->GetObject<Ipv4>()->GetAddress(1,0).GetLocal(); 
		Address clientAddress (InetSocketAddress (ipClient, DESTINATION_PORT_BASE + flowIdx));

		MyOnOffHelper clientHelper (protocol, clientAddress);//?
		clientHelper.SetAttribute ("PacketSize", UintegerValue (config->appPayloadBytes));
		clientHelper.SetAttribute ("Remote", destinAddrValue);
		clientHelper.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));

		//Headers total 66.0 Bytes for UDP
		//UDP header: 8 Bytes
		//IP header: 20 Bytes
		//LLC snap header: 8 Bytes
		//MAC header: 26 Bytes
		//FCS: 4 Bytes
		double macPduBytes;
		double appRate;

		if(protocol == "ns3::UdpSocketFactory")
		{
			macPduBytes = config->appPayloadBytes + 36.0;
			appRate = config->flowsDmd.at(flowIdx) * config->appPayloadBytes / macPduBytes;
		}
		else if (protocol == "ns3::TcpSocketFactory")
		{
			macPduBytes = 1460.0 + 90.0; 
			appRate = config->flowsDmd.at(flowIdx) * 1460.0 / macPduBytes;

		}

		NS_LOG_INFO("Flow "<< flowIdx << " demand (MAC)" << config->flowsDmd.at(flowIdx) << " app rate "<< appRate);

		std::ostringstream oss;
		oss << "ns3::ConstantRandomVariable[Constant="
			<< (config->appPayloadBytes * 8.0) /(appRate)* 1e3
			<< "]";
		clientHelper.SetAttribute("Interval", StringValue (oss.str()));

		ApplicationContainer clientApp = clientHelper.Install (config->meshNodes->Get(config->flowsPath.at(flowIdx).front()));

		clientApp.Start (Seconds (1));
		clientApp.Stop (Seconds (config->simulationTime + 1));
		config->clients->Add(clientApp);

		Address destinAddress (InetSocketAddress (ipServer, DESTINATION_PORT_BASE + flowIdx));
		PacketSinkHelper serverHelper (protocol, destinAddress);
		ApplicationContainer serverApp = serverHelper.Install (config->meshNodes->Get(config->flowsPath.at(flowIdx).back()));

		serverApp.Start (Seconds (0));
		serverApp.Stop (Seconds (config->simulationTime + 1));
		config->servers->Add(serverApp);
	}
}

int main (int argc, char *argv[])
{
	LogComponentEnable("DmgSimple", LOG_LEVEL_INFO);
	//LogComponentEnable("DmgAlmightyController", LOG_LEVEL_ALL);

	Config::SetDefault ("ns3::TcpL4Protocol::SocketType", TypeIdValue (TcpNewReno::GetTypeId ()));
	Config::SetDefault ("ns3::TcpSocket::SegmentSize", UintegerValue (1460));
	//TcpSocket maximum transmit buffer size (bytes) default 131072(128k)
	Config::SetDefault ("ns3::TcpSocket::SndBufSize", UintegerValue (10000000));
	Config::SetDefault ("ns3::TcpSocket::RcvBufSize", UintegerValue (10000000));
	Config::SetDefault ("ns3::TcpSocket::InitialSlowStartThreshold", UintegerValue (5000000));//5000000//65535 by default

//	Config::SetDefault ("ns3::TcpSocketBase::MaxWindowSize", UintegerValue (655350));//will be scaled to 1073725440 (14 bits to the left)
	/* By default the simulation lasts for 10 seconds */
	double simulationTime = 50;
	/*Simulation scenario*/
	int16_t scenario = 1;
	/* By default the simulation is the NS-3 default */
	int32_t rngSeed = 1;
	/* By default there are 4 mesh nodes */
	uint32_t meshNodesN = 4;
	/* By default there is one central controller */
	//uint32_t ctrlNodesN = 1;
	/* By default pcap tracing is disabled */
	bool enablePcap = false;
	/* By default UDP payload is 1470 bytes */
	uint32_t payloadSize = 1470;
	/* By default nodes are placed in a square area of
	 * square_dise_l x square_side_l meters */
	double square_side_l = 20;
	/* By default frame min BER is 0 (The real values depends on the relative
	 * positions of the nodes */
	double frameMinBer = 0;
	/* Operating frequency in Hz */
	double freq = 59.4e9;
	/* By default Tx and Rx antenna gain in 15 dBi */
	double txRxAntennaGainDbi =  20; //15;
	/* By default the Tx and Rx gain is 1 dBi */
	double txRxGainDbi = 1.0;
	/* By default the Tx power is 10 dBi */
	double txPowerDbi = 10;
	/* By default Beacon Interval duration is 102.4ms */
	uint64_t biDurationNs = 1024000*100;
	/* By default 10% of the Beacon Interval is considered overhead */
	double biOverheadFraction = 0.1;
	/* By default 100ns is considered as the overhead for beam switching*/
	uint64_t beamSwitchOverhead = 0;
	/* By default progressive filling will be re-run after 5 biDuration*/
	uint64_t refillInterval = biDurationNs*5;
	/* By default OFDM phy is used */
	bool dmgOfdm = true;
	/* By default udp traffic is used */
	std::string trafficType = "udp";//
	/* By default the mean flow demand is 10000Mb/s*/
	std::vector <double> flowsDmd (1, 10000);
	/* By default the progressive filling step lenth is 0.001*/
	double proFillStepL = 0.001;
	/* By default the throughput will be printed into a file*/
	bool ifPrintThrpt = true;
	/* Max # aggregated MPDU */
	uint32_t nMpdus = 30;
	/*By default the mac queue capacity is 5000 */
	uint32_t macQueueSizeinPkts = 5000;
	/*By default there are 5 schedule per Bi*/
	uint32_t numSchedulePerBi = 20;
	/*By default the fraction of time used for traffic of inverse direction (e.g. tcp ack) is 0*/
	double ackTraffFrac = 0.06;//94/(1554+94)= 0.057038835

	std::string inputFileName ="scratch/2flows.txt";//"Fig4_inverse.txt";//


	/******************************
	 * Command line args parsing
	 *****************************/
	CommandLine cmd;
	cmd.AddValue("payloadSize", "UDP Payload size in bytes", payloadSize);
	cmd.AddValue("simulationTime", "Simulation time in seconds", simulationTime);
	cmd.AddValue("scenario", "Simulation scenario 5: Link rate degrades between sta 0 and 3", scenario);
	cmd.AddValue("meshNodesN", "Number of mesh nodes", meshNodesN);
	cmd.AddValue("pcap", "true for pcap capture", enablePcap);
	cmd.AddValue("seed", "RNG seed", rngSeed);
	cmd.AddValue("MinBer", "Minimum bit error rate", frameMinBer);
	cmd.AddValue("gainAntennaDbi", "TX and RX antenna gain (dBi)", txRxAntennaGainDbi);
	cmd.AddValue("gainDbi", "TX and RX gain (dBI)", txRxGainDbi);
	cmd.AddValue("txPowerDbi", "Tx power (dBi)", txPowerDbi);
	cmd.AddValue("biDur", "Beacon Interval duration in ns", biDurationNs);
	cmd.AddValue("biOH", "Beacon Interval overhead fraction", biOverheadFraction);
	cmd.AddValue("dmgOfdm", "True to enable OFDM MCSs, false to use SC MCSs", dmgOfdm);
	cmd.AddValue("nMpdus", "Number of aggregated MPDUs", nMpdus);
	cmd.AddValue("macQueueSizeinPkts", "Max number of packets allowed by mac queue", macQueueSizeinPkts);
	cmd.AddValue("inputFileName", "The name of txt file to read", inputFileName);
	cmd.AddValue("trafficType", "The traffic type to be simulated", trafficType);
	cmd.AddValue("proFillStepL", "The steplength used by the Progressive Filling algorithm",proFillStepL);
	cmd.AddValue("beamSwitchOverhead","Time in Nano seconds that represent the time consumed when switch beam to different destinations", beamSwitchOverhead);
	cmd.AddValue("refillInterval","Time interval in Nano seconds to rerun progressive filling", refillInterval);
	cmd.AddValue("ifPrintThroughput","Print throughput in file or not", ifPrintThrpt);
	cmd.AddValue("numSchedulePerBi","Number of Schedules Per Beacon Interval", numSchedulePerBi);
	cmd.AddValue("ackTraffFrac","The fraction of time used for traffic of inverse direction (e.g. tcp ack)", ackTraffFrac);
	cmd.Parse (argc, argv);


	config.meshNodesN = meshNodesN;
	config.meshNodes = new NodeContainer();
	config.dmgCtrl = CreateObject<DmgAlmightyController> ();
	config.appPayloadBytes = payloadSize;
	config.enablePcap = enablePcap;
	config.simulationTime = simulationTime;
	config.scenario = scenario;
	config.square_side_l = square_side_l;
	config.frameMinBer = frameMinBer;
	config.txRxAntennaGainDbi = txRxAntennaGainDbi;
	config.txPowerDbi = txPowerDbi;
	config.txRxGainDbi = txRxGainDbi;
	config.biDurationNs = biDurationNs;
	config.biOverheadFraction = biOverheadFraction;
	config.refillInterval = refillInterval;
	config.dmgOfdm = dmgOfdm;
	if (nMpdus > 64) {
		nMpdus = 64;
	}
	config.nMpdus = nMpdus;
	config.servers = new ApplicationContainer();
	config.clients = new ApplicationContainer();

	config.lastReportTime = new Time(MicroSeconds(0));
	config.inputFileName =inputFileName;
	config.trafficType = trafficType;
	config.macQueueSizeinPkts = macQueueSizeinPkts;



	/* File stream for throughput report.
	 * By default saved in the current working directory with the name "tp"
	 */
	if(config.trafficType == "udp")
		config.dir_oss << "output/scenario"  << config.scenario << "/udp/";
	else if (config.trafficType == "tcp")		
		config.dir_oss << "output/scenario"  << config.scenario << "/tcp/";
		NS_LOG_INFO("output path: " << config.dir_oss.str());


	ParsingTopologyFromFile(&config);

	config.proFillStepL = proFillStepL;
	config.ifPrintThrpt = ifPrintThrpt;
	config.numSchedulePerBi = numSchedulePerBi;
	config.ackTraffFrac = (config.trafficType == "udp")?(0.0):ackTraffFrac;

	/******************************
	 * Set Seed and random flow demands
	 *****************************/
	if (rngSeed >= 0){
		RngSeedManager::SetSeed (rngSeed);
		RngSeedManager::SetRun (7);   // Changes run number from default of 1 to 7

	}

	/******************************
	 * Disable RTS/CTS
	 *****************************/
	Config::SetDefault ("ns3::WifiRemoteStationManager::RtsCtsThreshold",
			StringValue ("999999"));

	/******************************
	 * Disable fragmentation
	 *****************************/
	Config::SetDefault ("ns3::WifiRemoteStationManager::FragmentationThreshold",
			StringValue ("990000"));
	/******************************
	 * Maximum queue size
	 *****************************/
	Config::SetDefault ("ns3::WifiMacQueue::MaxPacketNumber", UintegerValue (config.macQueueSizeinPkts));

	/******************************
	 * Create mesh and controller nodes
	 *****************************/
	config.meshNodes->Create(config.meshNodesN);

	/******************************
	 * Yans wifi and channel configuration 
	 *****************************/
	YansWifiChannelHelper channelHelper;
	channelHelper.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
	/* Friis model with standard-specific wavelength */
	channelHelper.AddPropagationLoss ("ns3::FriisLoSPropagationLossModel",
			"Frequency", DoubleValue(freq));

	YansWifiPhyHelper phy = YansWifiPhyHelper::Default ();
	phy.Set("dmgOfdm", BooleanValue(config.dmgOfdm));
	Ptr<YansWifiChannel> channel = channelHelper.Create();
	phy.SetChannel (channel);
	phy.SetErrorRateModel("ns3::SensitivityModel60GHz");

	/******************************
	 * MAC configuration 
	 *****************************/
	WifiHelper wifi = WifiHelper::Default ();
	wifi.SetStandard (WIFI_PHY_STANDARD_80211ad);

	wifi.SetRemoteStationManager ("ns3::DmgDestinationFixedWifiManager");

	Ssid ssid = Ssid (SSID_STR);
	DmgWifiMacHelper meshMac = DmgWifiMacHelper::Default ();

	NetDeviceContainer devices;

	meshMac.SetType ("ns3::DmgWifiMac");

	if (config.nMpdus > 0) {
		meshMac.SetBlockAckThresholdForAc (AC_BE, 1);

		meshMac.SetMpduAggregatorForAc (AC_BE,"ns3::MpduStandardAggregator",
				"MaxAmpduSize", UintegerValue (config.nMpdus * (config.appPayloadBytes + 100)));
	}

	devices = wifi.Install(phy, meshMac, *(config.meshNodes));

	/******************************
	 * Set node positions 
	 *****************************/
	MobilityHelper mobility;
	Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
	mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");

	for (uint32_t i = 0; i < (config.meshNodesN); i++)
	{
		/* nodes are placed inside a square area of
		 * square_side_l x square_side_l meters */

		Vector posNode (config.nodesPosition[i][0],
				config.nodesPosition[i][1],
				0);

		positionAlloc->Add(posNode);
	}

	mobility.SetPositionAllocator (positionAlloc);
	mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
	mobility.Install (*(config.meshNodes));

	/******************************
	 * Internet stack configuration
	 *****************************/
	InternetStackHelper stack;

	Ipv4StaticRoutingHelper staticRouting;
	stack.SetRoutingHelper (staticRouting);  // has effect on the next Install ()

	stack.Install (*(config.meshNodes));

	Ipv4AddressHelper address;
	address.SetBase ("192.168.1.0", "255.255.255.0");
	Ipv4InterfaceContainer interfaces = address.Assign (devices);

	for (uint32_t flowIdx = 0; flowIdx< config.flowsPath.size(); flowIdx++){
		Ipv4Address destAddr = config.meshNodes->Get(config.flowsPath.at(flowIdx).back())->GetObject<Ipv4>()->GetAddress(1,0).GetLocal();
		Ipv4StaticRoutingHelper ipv4RoutingHelper;
		//Configure route from flowsSrc to flowsSink
		for (uint32_t staIdx = 0; staIdx< config.flowsPath.at(flowIdx).size() - 1; staIdx++){
			Ptr<Ipv4StaticRouting> staticRouting = ipv4RoutingHelper.GetStaticRouting
				(config.meshNodes->Get(config.flowsPath.at(flowIdx).at(staIdx))->GetObject<Ipv4> ());
			staticRouting->AddHostRouteTo (destAddr,config.meshNodes->Get(config.flowsPath.at(flowIdx).at(staIdx + 1))->GetObject<Ipv4>()->GetAddress(1,0).GetLocal(), 1);
		}
		destAddr = config.meshNodes->Get(config.flowsPath.at(flowIdx).front())->GetObject<Ipv4>()->GetAddress(1,0).GetLocal();
		//Configure route from flowsSink to flowsSrc
		for (uint32_t staIdx = config.flowsPath.at(flowIdx).size() - 1 ; staIdx > 0 ; staIdx--){
			Ptr<Ipv4StaticRouting> staticRouting = ipv4RoutingHelper.GetStaticRouting
				(config.meshNodes->Get(config.flowsPath.at(flowIdx).at(staIdx))->GetObject<Ipv4> ());
			staticRouting->AddHostRouteTo (destAddr,config.meshNodes->Get(config.flowsPath.at(flowIdx).at(staIdx - 1))->GetObject<Ipv4>()->GetAddress(1,0).GetLocal(), 1);
		}
	}

	//Print Routing Table
	Ipv4StaticRoutingHelper routingHelper;
	  Ptr<OutputStreamWrapper> routingStream = Create<OutputStreamWrapper>
	  ("routing.routes", std::ios::out);
	  routingHelper.PrintRoutingTableAllAt (Seconds (1), routingStream);
	
	/******************************
	 * Pcap tracing configuration
	 *****************************/
	if (config.enablePcap)
	{
		phy.SetPcapDataLinkType(YansWifiPhyHelper::DLT_IEEE802_11_RADIO); 
		for (uint32_t i = 0; i < config.meshNodes->GetN(); i++)
		{
			phy.EnablePcap("cap", config.meshNodes->Get(i)->GetDevice(0), true);
		}
	}

	/******************************
	 * Finalize configuration and start simulation 
	 *****************************/
	SetupDmgNodes (&config);
	SetupDmgController(&config);

	if (config.trafficType == "udp")
	{
		SetupOnOffTraffic(&config, "ns3::UdpSocketFactory");
	}
	else if (config.trafficType == "tcp")
	{
		SetupOnOffTraffic(&config, "ns3::TcpSocketFactory");
	}
	else
		NS_LOG_ERROR("Unknown transport protocol");


	printSimInfo();

	Simulator::Schedule(Seconds(0.9999999), FinalizeConfig, &config);

	AsciiTraceHelper ascii;

	if (config.ifPrintThrpt == true){
		Simulator::Schedule(NanoSeconds(config.biDurationNs * 10), CreateTpFiles, &config);
		Simulator::Schedule(NanoSeconds(config.biDurationNs * 11), WriteTpFiles, &config);
	}

	std::ostringstream mqFileName_oss;
	mqFileName_oss << config.dir_oss.str() << "mac-queue-size.txt";

	Ptr<OutputStreamWrapper> queuesize_stream =
		ascii.CreateFileStream (mqFileName_oss.str ());

	Simulator::Schedule(NanoSeconds(config.biDurationNs * 9), PrintQueueSize, &config, queuesize_stream);

	Simulator::Schedule(NanoSeconds(config.biDurationNs * 11 + 1), ReconfigureDmgBeaconInterval, &config, 1);

	Simulator::Stop (Seconds(config.simulationTime + 1));
	Simulator::Run ();

	Simulator::Destroy ();

	return 0;
}
