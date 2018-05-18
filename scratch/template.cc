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
#include "ns3/ipv4-static-routing-helper.h"
#include "ns3/flow-monitor.h"
#include "ns3/flow-monitor-helper.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/dmg-almighty-controller.h"
#include "ns3/buildings-module.h"

using namespace ns3;

#define SSID_STR		"test"
#define DESTINATION_PORT_BASE	20000
#define NUM_SUB_FLOWS	100

NS_LOG_COMPONENT_DEFINE ("DmgSimple");


	static void
CwndTracer (Ptr<OutputStreamWrapper> stream, uint32_t oldval, uint32_t newval)
{
	*stream->GetStream () << Simulator::Now ().GetSeconds () << " " << newval << std::endl;
}


	static void
SsThreshTracer (Ptr<OutputStreamWrapper> stream, uint32_t oldval, uint32_t newval)
{
	NS_LOG_INFO("Slow start threshold changed from "<< oldval <<" to "<< newval);
	*stream->GetStream () << Simulator::Now ().GetSeconds () << " " << newval << std::endl;
}


	static void
RttTracer (Ptr<OutputStreamWrapper> stream, Time oldval, Time newval)
{
//if((newval - oldval).GetMicroSeconds() > 100)
	*stream->GetStream () << Simulator::Now ().GetSeconds () << " " << newval.GetSeconds () << std::endl;
//	*stream->GetStream () << newval.GetMilliSeconds () << std::endl;
}

	static void
RtoTracer (Ptr<OutputStreamWrapper> stream, Time oldval, Time newval)
{
	*stream->GetStream () << Simulator::Now ().GetSeconds () << " " << newval.GetSeconds () << std::endl;
}

	static void
RwndTracer (Ptr<OutputStreamWrapper> stream, uint32_t oldval, uint32_t newval)
{
	*stream->GetStream () << Simulator::Now ().GetSeconds () << " " << newval << std::endl;
}

	static void
HseqTracer (Ptr<OutputStreamWrapper> stream, SequenceNumber32 oldval, SequenceNumber32 newval)
{
	*stream->GetStream () << Simulator::Now ().GetSeconds () << " " << newval << std::endl;
}

	static void
UnackSeqTracer (Ptr<OutputStreamWrapper> stream, SequenceNumber32 oldval, SequenceNumber32 newval)
{
	*stream->GetStream () << Simulator::Now ().GetSeconds () << " " << newval << std::endl;
}

	static void
RxSeqTracer (Ptr<OutputStreamWrapper> stream, SequenceNumber32 oldval, SequenceNumber32 newval)
{
	*stream->GetStream () << Simulator::Now ().GetSeconds () << " " << newval << std::endl;
}


	static void
ConnectTcpTracers (std::string fileNameBase, std::vector < std::vector <uint32_t> > flowPath, uint16_t scenario)
{
	fileNameBase.append("subflows/");
	//DL flow for now, flows share the same source, hence the socket index increases. sinks varies -> socket remains for RxSeqTracer
	for (uint32_t flowIdx=0; flowIdx< flowPath.size(); flowIdx++)
	{		
		for (uint32_t subf = 0; subf < NUM_SUB_FLOWS; subf= subf + 1)
		{
			std::ostringstream tail;
			uint32_t socketId;
			if(scenario !=3)
			{
				//naming format: cwnd-flow.dat
				tail<< flowIdx <<".dat";

				socketId = flowIdx;
			}
			else 
			{
				//naming format: cwnd-flow-subflow.dat
				tail << flowIdx << "-sub"<< subf <<".txt"; 
				socketId = flowIdx * NUM_SUB_FLOWS + subf;
			}
			//CWND
			std::ostringstream fileName_oss;
			fileName_oss << fileNameBase << "cwnd-"  << tail.str();

			AsciiTraceHelper ascii;
			Ptr<OutputStreamWrapper> stream = ascii.CreateFileStream (fileName_oss.str ());

			std::ostringstream tracePath_oss;
/*			tracePath_oss << "/NodeList/"<< flowPath.at(flowIdx).front() <<"/$ns3::TcpL4Protocol/SocketList/"<< socketId <<"/CongestionWindow" ;

			Config::ConnectWithoutContext (tracePath_oss.str (), MakeBoundCallback (&CwndTracer, stream));
*/
			//RTT
			fileName_oss.str("");  //		fileName_oss.clear();
			tracePath_oss.str(""); //		tracePath_oss.clear();

			fileName_oss << fileNameBase << "rtt-"  << tail.str();
			stream = ascii.CreateFileStream (fileName_oss.str ());
			tracePath_oss << "/NodeList/"<< flowPath.at(flowIdx).front() <<"/$ns3::TcpL4Protocol/SocketList/"<< socketId <<"/RTT" ;
			Config::ConnectWithoutContext (tracePath_oss.str (), MakeBoundCallback (&RttTracer, stream));

/*
			//SsThresh
			fileName_oss.str("");  //		fileName_oss.clear();
			tracePath_oss.str(""); //		tracePath_oss.clear();

			fileName_oss << fileNameBase << "ssthresh-"  << tail.str();
			stream = ascii.CreateFileStream (fileName_oss.str ());
			tracePath_oss << "/NodeList/"<< flowPath.at(flowIdx).front() <<"/$ns3::TcpL4Protocol/SocketList/"<< socketId <<"/SlowStartThreshold" ;
			Config::ConnectWithoutContext (tracePath_oss.str (), MakeBoundCallback (&SsThreshTracer, stream));

			//RTO
			fileName_oss.str("");  //		fileName_oss.clear();
			tracePath_oss.str(""); //		tracePath_oss.clear();

			fileName_oss << fileNameBase << "rto-"  << tail.str();  		
			stream = ascii.CreateFileStream (fileName_oss.str ());
			tracePath_oss << "/NodeList/"<< flowPath.at(flowIdx).front() <<"/$ns3::TcpL4Protocol/SocketList/"<< socketId <<"/RTO" ;
			Config::ConnectWithoutContext (tracePath_oss.str (), MakeBoundCallback (&RtoTracer, stream));


			//RwndTracer
			fileName_oss.str("");  //		fileName_oss.clear();
			tracePath_oss.str(""); //		tracePath_oss.clear();

			fileName_oss << fileNameBase << "rwnd-"  << tail.str();  		
			stream = ascii.CreateFileStream (fileName_oss.str ());
			tracePath_oss << "/NodeList/"<< flowPath.at(flowIdx).front() <<"/$ns3::TcpL4Protocol/SocketList/"<< socketId <<"/RWND" ;
			Config::ConnectWithoutContext (tracePath_oss.str (), MakeBoundCallback (&RwndTracer, stream));

			//RxSeqTracer --tcp rx buffer
			fileName_oss.str("");  //		fileName_oss.clear();
			tracePath_oss.str(""); //		tracePath_oss.clear();

			fileName_oss << fileNameBase << "rxseq-"  << tail.str();  		
			stream = ascii.CreateFileStream (fileName_oss.str ());
			//subf + 100 --> by accident
			tracePath_oss << "/NodeList/"<< flowPath.at(flowIdx).back() <<"/$ns3::TcpL4Protocol/SocketList/"<< subf + 100 << "/RxBuffer/NextRxSequence" ;
			Config::ConnectWithoutContext (tracePath_oss.str (), MakeBoundCallback (&RxSeqTracer, stream));

			//UnackSequence
			fileName_oss.str("");  //               fileName_oss.clear();
			tracePath_oss.str(""); //               tracePath_oss.clear();

			fileName_oss << fileNameBase << "unackseq-"  << tail.str();             
			stream = ascii.CreateFileStream (fileName_oss.str ());
			tracePath_oss << "/NodeList/"<< flowPath.at(flowIdx).front() <<"/$ns3::TcpL4Protocol/SocketList/"<< socketId <<"/TxBuffer/UnackSequence" ;
			Config::ConnectWithoutContext (tracePath_oss.str (), MakeBoundCallback (&UnackSeqTracer, stream));

			//HseqTracer: for the HighestSequence sent
			fileName_oss.str("");  //		fileName_oss.clear();
			tracePath_oss.str(""); //		tracePath_oss.clear();

			fileName_oss << fileNameBase << "hseq-"  << tail.str();  		
			stream = ascii.CreateFileStream (fileName_oss.str ());
			tracePath_oss << "/NodeList/"<< flowPath.at(flowIdx).front() <<"/$ns3::TcpL4Protocol/SocketList/"<< socketId << "/HighestSequence" ;
			Config::ConnectWithoutContext (tracePath_oss.str (), MakeBoundCallback (&HseqTracer, stream));
*/
			if(scenario !=3)
				break; //break from subflow loop
		}

	}

}


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

	/* Transmission rate at the application level in Mb/s */
	//double udpTxRate;

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
	std::string appFileName;
	std::map <uint32_t, double> rateProb;

	std::vector < uint32_t > m_nextHop;
	double proFillStepL;
	bool ifPrint;
	bool ifVbr;
    bool ifInterf;
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
	NS_LOG_INFO( "Nodes randomly placed in a square " <<
			config.square_side_l << "x" << config.square_side_l << " (m)" );

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

	NS_LOG_INFO("Topology read from file: " << config.appFileName);

	NS_LOG_INFO( "Progressive filling step length: " << config.proFillStepL << "Mb/s");

	NS_LOG_INFO( "Print throughput info in to a file: (Yes if 1)" << config.ifPrint);

	NS_LOG_INFO( "Use varaible data rate: " << config.ifVbr );

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

	/*for (uint32_t flowIdx = 0; flowIdx< config.flowsPath.size();flowIdx++){
	  NS_LOG_INFO( "Flow path:");
	  for (uint32_t linkIdx = 0; linkIdx< config.flowsPath.at(flowIdx).size(); linkIdx++){
	  NS_LOG_INFO( config.flowsPath.at(flowIdx).at(linkIdx));  
	  }
	  NS_LOG_INFO("Initial flow demand: " << config.flowsDmd.at(flowIdx) << " Mb/s.");
	  }*/

	NS_LOG_INFO("*********************************");
	NS_LOG_INFO("IDEAL RX POWER and MCS");
	NS_LOG_INFO("*********************************");
	config.dmgCtrl->PrintIdealRxPowerAndMcs();


	/*NS_LOG_INFO("*********************************" );
	  NS_LOG_INFO("SERVICE PERIODS");
	  NS_LOG_INFO("*********************************" );*/
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

		if(config->scenario == 3){
			for (uint32_t subf = 0; subf < NUM_SUB_FLOWS; subf++)
			{
				uint64_t subflowrx = DynamicCast<PacketSink>(config->servers->Get (flowIdx * NUM_SUB_FLOWS + subf))->GetTotalRx ();
				//NS_LOG_INFO(now_t<< " Flow "<< flowIdx <<" subflow "<< subf << " rx "<< subflowrx <<" in total");

				rx_bytes += subflowrx;//the total bytes received in this sink app 

				tx_bytes += DynamicCast<DashRateAdaptationApplication>(config->clients->Get (flowIdx * NUM_SUB_FLOWS + subf))->GetTotalTxBytes ();
			}
		}
		else
		{
			rx_bytes = DynamicCast<PacketSink>(config->servers->Get (flowIdx))->GetTotalRx ();//the total bytes received in this sink

			tx_bytes = DynamicCast<MyOnOffApplication>(config->clients->Get (flowIdx))->GetTotalTxBytes ();
		}

		double cur_rx_Mbits = (rx_bytes * 8.0) / 1e6;
		double cur_tx_Mbits = (tx_bytes * 8.0) / 1e6;


		//mac throughput
		Ipv4Address srcIp = config->meshNodes->Get(config->flowsPath.at(flowIdx).front())->GetObject<Ipv4>()->GetAddress(1,0).GetLocal();
		uint64_t rx_bytes_mac = config->meshNodes->Get(config->flowsPath.at(flowIdx).back())->GetDevice(0)->GetObject<WifiNetDevice>()->GetMac()->GetObject<DmgWifiMac>()->GetBytesRxFrom(srcIp);

		double cur_rx_Mbits_mac = (rx_bytes_mac * 8.0) / 1e6;

		NS_LOG_UNCOND(now_t << " flow " << flowIdx << "\t"<< config->flowsDmd.at(flowIdx) <<" "  << config->predictedFlowRate.at(flowIdx) << " rx rate: " << (cur_rx_Mbits - config->lastRxMbits[flowIdx]) / ((now_t - *(config->lastReportTime)).GetNanoSeconds() / 1e9) <<" Mbps. tx rate: "<< (cur_tx_Mbits - config->lastTxMbits[flowIdx]) / ((now_t - *(config->lastReportTime)).GetNanoSeconds() / 1e9));

		*config->streams_tp.at(flowIdx)->GetStream() << config->flowsDmd.at(flowIdx) <<"\t"  << config->predictedFlowRate.at(flowIdx) << "\t" << (cur_rx_Mbits - config->lastRxMbits[flowIdx]) / ((now_t - *(config->lastReportTime)).GetNanoSeconds() / 1e9) << "\t" << (cur_tx_Mbits - config->lastTxMbits[flowIdx]) / ((now_t - *(config->lastReportTime)).GetNanoSeconds() / 1e9)<< std::endl;

		config->lastRxMbits[flowIdx] = cur_rx_Mbits;
		config->lastTxMbits[flowIdx] = cur_tx_Mbits;
		config->lastMacRxMbits[flowIdx] = cur_rx_Mbits_mac;
	}
	config->dmgCtrl->PrintIdealRxPowerAndMcs();

	Simulator::Schedule(Seconds(1), WriteTpFiles, config);
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
		//NS_LOG_INFO ("Input Document ` style: "<< line);
		getline (topgen,line);
	}

	lineBuffer.str (line);
	lineBuffer >> config->meshNodesN;
	lineBuffer >> totflow;

	//NS_LOG_INFO ("Topology meta: " << config->meshNodesN << " nodes, " << totflow<<" flows");

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

		if ( (!xCoordinate.empty ()) && (!yCoordinate.empty ()) )
		{
			//NS_LOG_INFO ( "Node " << nodeIdx << " location x: " << config->nodesPosition[i][0] << " y: " << config->nodesPosition[i][1]);
		}
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

		//NS_LOG_DEBUG(" Flow "<< i <<" demand "<< config->flowsDmd.back() << "Mb/s. Path: ");

		for ( uint32_t j = 0; j < config->flowsPath.at(i).size(); j++){  
			//NS_LOG_DEBUG(" "<< config->flowsPath.at(i).at(j));
		}
	}
}

/* Parse the Inet format topology input file*/
	void
ParsingAppRateDistribution(struct sim_config *config)
{
	std::ifstream topgen;
	topgen.open(config->appFileName.c_str());
	if ( !topgen.is_open () )
	{
		NS_LOG_INFO ("file "<< config->appFileName<< " is not open, check file name and permissions");
		return;
	}

	std::string from;
	std::string value;

	uint32_t rate = 0;
	double prob = 0.0;

	std::istringstream lineBuffer;
	std::string line;

	getline (topgen,line);

	if (line.length() > 0 && line.find('#') != std::string::npos){
		//NS_LOG_INFO ("Input Document ` style: "<< line);
		getline (topgen,line);
	}

	//Read location info
	while(!topgen.eof ())
	{

		lineBuffer.str (line);
		lineBuffer >> rate;
		lineBuffer >> prob;
		config->rateProb.insert(std::pair<uint32_t, double> (rate, prob));

		NS_LOG_INFO ("rate " << rate << " probability " << prob );
		getline (topgen,line);
		lineBuffer.clear ();
		lineBuffer.str (line);
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

	if (config->scenario == 1){
		std::string protocol = "ns3::UdpSocketFactory";
		NS_LOG_INFO("rate 6 " << config->flowsDmd.at(6));
		config->flowsDmd.at(6) = config->flowsDmd.at(6) + 300.0;
		NS_LOG_INFO("increasing rate 6 to " << config->flowsDmd.at(6));

		for (uint32_t flowIdx = 0; flowIdx< config->flowsPath.size();flowIdx++)
		{

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

			if (config->ifVbr){
				std::ostringstream oss;
				oss << "ns3::ExponentialRandomVariable[Mean="
					<< (config->appPayloadBytes * 8.0) /(appRate)* 1e3
					<< "]";

				config->clients->Get(flowIdx)->SetAttribute ("Interval", StringValue (oss.str()));
			}

			else{//CBR
				std::ostringstream oss;
				oss << "ns3::ConstantRandomVariable[Constant="
					<< (config->appPayloadBytes * 8.0) /(appRate)* 1e3
					<< "]";
				config->clients->Get(flowIdx)->SetAttribute ("Interval", StringValue (oss.str()));
			}
		}
	}
	else if (config->scenario == 2){
		Vector pos = config->meshNodes->Get(5)->GetObject<MobilityModel>()->GetPosition();
		config->meshNodes->Get(5)->GetObject<MobilityModel>()->SetPosition(Vector (pos.x, pos.y - 10, pos.z));
		config->dmgCtrl->ConfigureWifiManager();
	}
	else if (config->scenario == 3){
		config->flowsDmd = config->dmgCtrl->GetMacEnqueueRateMeasurement();
	}
	else if (config->scenario == 4){
		if (it >= 2)
			config->dmgCtrl->EnforceAdditionalSignalLossBetween(config->meshNodes->Get(5), config->meshNodes->Get(8), 20 - (10 * (it-1)));
	}
	else if (config->scenario == 5){
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

	NS_LOG_INFO("*********************************");
	NS_LOG_INFO("SERVICE PERIODS");
	NS_LOG_INFO("*********************************");
	config->dmgCtrl->PrintSpInfo();

}

/* Utility function used to configure the DMG controller */
void SetupDmgController (struct sim_config *config)
{
	/* WARNING: in the current implementation it is suggested to call the
	 * functions used to configure the DmgALmightyController as in this example.
	 */
	/* We pass the container of the mesh nodes to the DmgAlmightyController */
    config->dmgCtrl->SetSimInterference(config->ifInterf);
	config->dmgCtrl->SetGw(0);
	config->dmgCtrl->SetMeshNodes(config->meshNodes);

	config->dmgCtrl->SetFlowsPath(config->flowsPath);

	config->dmgCtrl->ConfigureCliques();

	config->dmgCtrl->SetBeamSwitchOverhead(config->beamSwitchOverhead);

	/* Configure the DmgDestinationFixedWifiManager */
	config->dmgCtrl->ConfigureWifiManager(); 	

	if (config->scenario == 1)
	{
		config->flowsDmd.at(6) = 300.0;
	}
	else if (config->scenario == 4)
	{
		//blockage
		config->dmgCtrl->EnforceAdditionalSignalLossBetween(config->meshNodes->Get(5), config->meshNodes->Get(8), 20);
	}


	config->dmgCtrl->SetNumSchedulePerBi(config->numSchedulePerBi);

	config->dmgCtrl->SetAckTimeFrac(config->ackTraffFrac);

	config->predictedFlowRate = config->dmgCtrl->FlowRateProgressiveFilling(config->flowsDmd, config->proFillStepL, config->appPayloadBytes, config->biOverheadFraction, config->nMpdus);

	//config->predictedFlowRate = config->dmgCtrl->FlowRateMaxDlmac(config->appPayloadBytes, config->biOverheadFraction);
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

void StartDmdRateMeasurement (struct sim_config *config)
{
	config->dmgCtrl->StartMacEnqueueRateMeasurement();
}

void GetDmdRateMeasurement (struct sim_config *config)
{
	config->flowsDmd = config->dmgCtrl->GetMacEnqueueRateMeasurement();
	Simulator::Schedule(NanoSeconds(config->biDurationNs), GetDmdRateMeasurement, config);
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

		if (config->ifVbr){
			std::ostringstream oss;
			oss << "ns3::ExponentialRandomVariable[Mean="
				<< (config->appPayloadBytes * 8.0) /(appRate)* 1e3
				<< "]";

			clientHelper.SetAttribute("Interval", StringValue (oss.str()));
		}
		else{//CBR
			std::ostringstream oss;
			oss << "ns3::ConstantRandomVariable[Constant="
				<< (config->appPayloadBytes * 8.0) /(appRate)* 1e3
				<< "]";
			clientHelper.SetAttribute("Interval", StringValue (oss.str()));
		}

		ApplicationContainer clientApp = clientHelper.Install (config->meshNodes->Get(config->flowsPath.at(flowIdx).front()));
		//NS_LOG_DEBUG("install on off traffic client (src) in" << config->flowsPath.at(flowIdx).front() );

		clientApp.Start (Seconds (1));
		clientApp.Stop (Seconds (config->simulationTime + 1));
		config->clients->Add(clientApp);

		Address destinAddress (InetSocketAddress (ipServer, DESTINATION_PORT_BASE + flowIdx));
		PacketSinkHelper serverHelper (protocol, destinAddress);
		ApplicationContainer serverApp = serverHelper.Install (config->meshNodes->Get(config->flowsPath.at(flowIdx).back()));
		//NS_LOG_DEBUG("install on off traffic server (sink) on" << config->flowsPath.at(flowIdx).back() );
		serverApp.Start (Seconds (0));
		serverApp.Stop (Seconds (config->simulationTime + 1));
		config->servers->Add(serverApp);
	}
}

void SetupDashAdaptRateTraffic(struct sim_config *config, std::string protocol)
{	
	NS_LOG_INFO("setting "<< NUM_SUB_FLOWS << " subflows for DashRate app");	
	for (uint32_t flowIdx = 0; flowIdx< config->flowsPath.size(); flowIdx++)
	{
		for (uint32_t subf =0; subf < NUM_SUB_FLOWS; subf++)
		{

			Ipv4Address ipServer = config->meshNodes->Get(config->flowsPath.at(flowIdx).back())->GetObject<Ipv4>()->GetAddress(1,0).GetLocal();
			AddressValue destinAddrValue (InetSocketAddress (ipServer, DESTINATION_PORT_BASE + flowIdx*1000 + subf));     

			Ipv4Address ipClient = config->meshNodes->Get(config->flowsPath.at(flowIdx).front())->GetObject<Ipv4>()->GetAddress(1,0).GetLocal(); 
			Address clientAddress (InetSocketAddress (ipClient, DESTINATION_PORT_BASE + flowIdx*1000 + subf));

			DashRateAdaptationHelper clientHelper (protocol, clientAddress);//?
			//clientHelper.SetAttribute ("PacketSize", UintegerValue (config->appPayloadBytes));
			clientHelper.SetAttribute ("Remote", destinAddrValue);
			clientHelper.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
			clientHelper.SetAttribute ("Interval", StringValue ("ns3::ConstantRandomVariable[Constant=41666667]"));
			clientHelper.SetAttribute ("DataRateProbInterval", StringValue("ns3::UniformRandomVariable[Min=0.0,Max=1.0]"));

			ApplicationContainer clientApp = clientHelper.Install (config->meshNodes->Get(config->flowsPath.at(flowIdx).front()));
			//NS_LOG_INFO("install on off traffic client (src) in" << config->flowsPath.at(flowIdx).front() << " port "<< DESTINATION_PORT_BASE + flowIdx*1000 + subf );

			clientApp.Start (Seconds (1));// + NanoSeconds(subf));
			clientApp.Stop (Seconds (config->simulationTime + 1));
			clientApp.Get(0)->GetObject<DashRateAdaptationApplication>()->SetBitRateMap(config->rateProb);
			config->clients->Add(clientApp);

			Address destinAddress (InetSocketAddress (ipServer, DESTINATION_PORT_BASE + flowIdx*1000 + subf));
			PacketSinkHelper serverHelper (protocol, destinAddress);
			ApplicationContainer serverApp = serverHelper.Install (config->meshNodes->Get(config->flowsPath.at(flowIdx).back()));
			//NS_LOG_INFO("install on off traffic server (sink) on" << config->flowsPath.at(flowIdx).back() );
			serverApp.Start (Seconds (0));
			serverApp.Stop (Seconds (config->simulationTime + 1));
			config->servers->Add(serverApp);
		}
	}
}

void ConfigureBuildingBlockage (struct sim_config *config, Ptr<BuildingBlock> buildingBlock)
{
	for (uint32_t aSta = 0; aSta < config->meshNodesN; aSta++)
	{
		for (uint32_t bSta = 0; bSta < config->meshNodesN; bSta++)
		{
			if(aSta == bSta)
				continue;

			Ptr<YansWifiPhy> aPhy = config->meshNodes->Get(aSta)->GetDevice(0)->GetObject<WifiNetDevice>()->GetPhy()->GetObject<YansWifiPhy> ();
			Ptr<YansWifiPhy> bPhy = config->meshNodes->Get(bSta)->GetDevice(0)->GetObject<WifiNetDevice>()->GetPhy()->GetObject<YansWifiPhy> ();

			Ptr<MobilityModel> aMob = config->meshNodes->Get(aSta)->GetObject<MobilityModel> ();
			Ptr<MobilityModel> bMob = config->meshNodes->Get(bSta)->GetObject<MobilityModel> ();

			if(buildingBlock->GetIfLos(aMob,bMob))
			{
				aPhy->GetChannel()->GetObject<YansWifiChannel>()->GetPropagationLossModel()->GetObject<FriisLoSPropagationLossModel>()->SetLoS(aMob,bMob,true);
				bPhy->GetChannel()->GetObject<YansWifiChannel>()->GetPropagationLossModel()->GetObject<FriisLoSPropagationLossModel>()->SetLoS(aMob,bMob,true);
			}
			else
			{
				NS_LOG_DEBUG("NLOS between STAs "<< aSta <<" and "<< bSta);

				aPhy->GetChannel()->GetObject<YansWifiChannel>()->GetPropagationLossModel()->GetObject<FriisLoSPropagationLossModel>()->SetLoS(aMob,bMob,false);
				bPhy->GetChannel()->GetObject<YansWifiChannel>()->GetPropagationLossModel()->GetObject<FriisLoSPropagationLossModel>()->SetLoS(aMob,bMob,false);
			}
		}
	}
}

int main (int argc, char *argv[])
{
    LogComponentEnable("DmgSimple", LOG_LEVEL_INFO);
	Config::SetDefault ("ns3::TcpL4Protocol::SocketType", TypeIdValue (TcpNewReno::GetTypeId ()));
	Config::SetDefault ("ns3::TcpSocket::SegmentSize", UintegerValue (1460));
	//TcpSocket maximum transmit buffer size (bytes) default 131072(128k)
	Config::SetDefault ("ns3::TcpSocket::SndBufSize", UintegerValue (10000000));
	Config::SetDefault ("ns3::TcpSocket::RcvBufSize", UintegerValue (10000000));
	Config::SetDefault ("ns3::TcpSocket::InitialSlowStartThreshold", UintegerValue (65535));//5000000//65535 by default

	//Config::SetDefault ("ns3::TcpSocketBase::MaxWindowSize", UintegerValue (65535));//will be scaled to 1073725440 (14 bits to the left)
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
	bool ifPrint = true;

	bool ifVbr = false;
    bool ifInterf = true;
	/* Max # aggregated MPDU */
	uint32_t nMpdus = 30;
	/*By default the mac queue capacity is 5000 */
	uint32_t macQueueSizeinPkts = 5000;
	/*By default there are 5 schedule per Bi*/
	uint32_t numSchedulePerBi = 20;
	/*By default the fraction of time used for traffic of inverse direction (e.g. tcp ack) is 0*/
	double ackTraffFrac = 0.06;//94/(1554+94)= 0.057038835

	std::string inputFileName ="scratch/Nottin.txt";//"Fig4_inverse.txt";//

	std::string appFileName="scratch/histo_value.txt";

	/******************************
	 * Command line args parsing
	 *****************************/
	CommandLine cmd;
	cmd.AddValue("payloadSize", "UDP Payload size in bytes", payloadSize);
	cmd.AddValue("simulationTime", "Simulation time in seconds", simulationTime);
	cmd.AddValue("scenario", "Simulation scenario 0: no network dynamics \n 1: Flow 8 demand increasing \n 2: (Fig 5) distance from 5-3 decreasing capa link 3-5 increases \n 3: measured demand \n 4: Link rate degrades between sta 4 and 16", scenario);
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
	cmd.AddValue("ifPrintThroughput","Print throughput in file or not", ifPrint);
	cmd.AddValue("ifVbr","Use VBR", ifVbr);
    cmd.AddValue("ifInterf","Simulate interference", ifInterf);
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
	config.appFileName =appFileName;
	config.trafficType = trafficType;
	config.macQueueSizeinPkts = macQueueSizeinPkts;
    config.ifInterf = ifInterf;


	/* File stream for throughput report.
	 * By default saved in the current working directory with the name "tp"
	 */

	if(config.scenario > 10 && config.scenario <= 20)
	{
		//read from 
		//write to --> topo*/scenario*/mpdu*/
		uint32_t topology =  config.scenario - 10;
		NS_LOG_INFO("input topology: " << topology);
		//overwrite the user input of 'inputFileName'
		//from 'scratch/xxx' to 'topo2/topo2.txt'
		std::ostringstream input_oss;
		input_oss << "topo" << topology << "/topo" << topology  << ".txt";
		NS_LOG_INFO("input path: " << input_oss.str()); 
		config.inputFileName = input_oss.str();
		config.dir_oss << "topo"<< topology << "/scenario"  << config.scenario << "/mpdu"<< config.nMpdus << "/";
		NS_LOG_INFO("output path: " << config.dir_oss.str());
	}
	else if(config.scenario <= 10)
	{
		if(config.trafficType == "udp")
			config.dir_oss << "topoNottin/scenario"  << config.scenario << "/udp/";
		else if (config.trafficType == "tcp")		
			config.dir_oss << "topoNottin/scenario"  << config.scenario << "/tcp/";
		NS_LOG_INFO("output path: " << config.dir_oss.str());
	}

	ParsingTopologyFromFile(&config);

	if(config.scenario == 3)
		ParsingAppRateDistribution(&config);

	config.proFillStepL = proFillStepL;
	config.ifPrint = ifPrint;
	config.ifVbr = ifVbr;
//	config.numSchedulePerBi = (config.trafficType == "udp")?numSchedulePerBi:(numSchedulePerBi - 5);
	config.numSchedulePerBi = numSchedulePerBi;
	config.ackTraffFrac = (config.trafficType == "udp")?(0.0):ackTraffFrac;


	/*Uniform Random Variable*/
	//Ptr<UniformRandomVariable> unif_rng = CreateObject<UniformRandomVariable> ();
	/*Normal Random Variable*/
	/*config.rng = CreateObject<NormalRandomVariable> ();
	  config.rng->SetAttribute ("Mean", DoubleValue (config.flowDmdMax/2));
	  config.rng->SetAttribute ("Variance", DoubleValue (1/2));*/
	/******************************
	 * Set Seed and random flow demands
	 *****************************/

	//rngSeed = 1;
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
			//NS_LOG_DEBUG("Flow "<<flowIdx<< " destin "<< destAddr << " sta " << config.flowsPath.at(flowIdx).at(staIdx)<<" next hop "<< config.flowsPath.at(flowIdx).at(staIdx+1));
			Ptr<Ipv4StaticRouting> staticRouting = ipv4RoutingHelper.GetStaticRouting
				(config.meshNodes->Get(config.flowsPath.at(flowIdx).at(staIdx))->GetObject<Ipv4> ());
			staticRouting->AddHostRouteTo (destAddr,config.meshNodes->Get(config.flowsPath.at(flowIdx).at(staIdx + 1))->GetObject<Ipv4>()->GetAddress(1,0).GetLocal(), 1);
		}
		destAddr = config.meshNodes->Get(config.flowsPath.at(flowIdx).front())->GetObject<Ipv4>()->GetAddress(1,0).GetLocal();
		//Configure route from flowsSink to flowsSrc
		for (uint32_t staIdx = config.flowsPath.at(flowIdx).size() - 1 ; staIdx > 0 ; staIdx--){
			//NS_LOG_DEBUG("Flow "<<flowIdx<< "_inverse destin "<< destAddr << " sta " << config.flowsPath.at(flowIdx).at(staIdx)<<" next hop "<< config.flowsPath.at(flowIdx).at(staIdx - 1));
			Ptr<Ipv4StaticRouting> staticRouting = ipv4RoutingHelper.GetStaticRouting
				(config.meshNodes->Get(config.flowsPath.at(flowIdx).at(staIdx))->GetObject<Ipv4> ());
			staticRouting->AddHostRouteTo (destAddr,config.meshNodes->Get(config.flowsPath.at(flowIdx).at(staIdx - 1))->GetObject<Ipv4>()->GetAddress(1,0).GetLocal(), 1);
		}
	}

	//Print Routing Table
	/*Ipv4StaticRoutingHelper routingHelper;
	  Ptr<OutputStreamWrapper> routingStream = Create<OutputStreamWrapper>
	  ("routing.routes", std::ios::out);
	  routingHelper.PrintRoutingTableAllAt (Seconds (1), routingStream);
	 */
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
	 * Add buildings
	 *****************************/
	/*
	   Ptr<BuildingBlock> buildingBlock = CreateObject<BuildingBlock> ();

	   Ptr<Building> building;

	   building = CreateObject<Building> ();
	   building->SetBoundaries (Box (2.0, 100.0,
	   -0.0, 30.0,
	   0.0, 10.0));
	   buildingBlock->AddBuilding(building);

	   building = CreateObject<Building> ();
	   building->SetBoundaries (Box (40.0, 100.0,
	   -150.0, -70.0,
	   0.0, 10.0));

	   buildingBlock->AddBuilding(building);

	   if(config.inputFileName=="scratch/widiNottin.txt")
	   ConfigureBuildingBlockage(&config, buildingBlock);*/

	/******************************
	 * Finalize configuration and start simulation 
	 *****************************/
	SetupDmgNodes (&config);
	SetupDmgController(&config);

	if (config.trafficType == "udp")
	{
		if(config.scenario == 3)
			SetupDashAdaptRateTraffic(&config, "ns3::UdpSocketFactory");
		else
			SetupOnOffTraffic(&config, "ns3::UdpSocketFactory");
	}
	else if (config.trafficType == "tcp")
	{
		//config.dir_oss << "tcp/";

		if(config.scenario == 3)
			SetupDashAdaptRateTraffic(&config, "ns3::TcpSocketFactory");//TODO: check this func for tcp
		else
			SetupOnOffTraffic(&config, "ns3::TcpSocketFactory");
		//temp 
		Simulator::Schedule (Seconds (1.01), &ConnectTcpTracers, config.dir_oss.str(), config.flowsPath, config.scenario);
	}
	else
		NS_LOG_ERROR("Unknown transport protocol");


	printSimInfo();

	Simulator::Schedule(Seconds(0.9999999), FinalizeConfig, &config);

	AsciiTraceHelper ascii;

	if (config.ifPrint == true){
		Simulator::Schedule(NanoSeconds(config.biDurationNs * 10), CreateTpFiles, &config);
		Simulator::Schedule(NanoSeconds(config.biDurationNs * 11), WriteTpFiles, &config);
	}

	std::ostringstream mqFileName_oss;
	mqFileName_oss << config.dir_oss.str() << "mac-queue-size.txt";

	Ptr<OutputStreamWrapper> queuesize_stream =
		ascii.CreateFileStream (mqFileName_oss.str ());

	Simulator::Schedule(NanoSeconds(config.biDurationNs * 9), PrintQueueSize, &config, queuesize_stream);

	if(config.scenario == 3){
		Simulator::Schedule(NanoSeconds(config.biDurationNs * 10), StartDmdRateMeasurement, &config);
		Simulator::Schedule(NanoSeconds(config.biDurationNs * 11), GetDmdRateMeasurement, &config);
	}
	else if (config.scenario > 0 && config.scenario <= 10)
		Simulator::Schedule(NanoSeconds(config.biDurationNs * 13 + 1), ReconfigureDmgBeaconInterval, &config, 1);
	else if (config.scenario > 10 && config.scenario <= 20)
		Simulator::Schedule(NanoSeconds(config.biDurationNs * 11 + 1), ReconfigureDmgBeaconInterval, &config, 1);

	FlowMonitorHelper flowmon;
	Ptr<FlowMonitor> monitor =flowmon.InstallAll ();

	Simulator::Stop (Seconds(config.simulationTime + 1));
	Simulator::Run ();

	//monitor->SerializeToXmlFile ("results.xml",true,false);//True for histogram false for probe
	Simulator::Destroy ();

	return 0;
}
