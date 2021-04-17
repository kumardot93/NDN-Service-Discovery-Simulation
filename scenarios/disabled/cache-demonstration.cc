// ndn-simple.cpp

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/ndnSIM-module.h"


#include "ns3/log.h"
#include "ns3/string.h"
#include "ns3/uinteger.h"
#include "ns3/packet.h"
#include "ns3/simulator.h"

#include "model/ndn-l3-protocol.hpp"
#include "helper/ndn-fib-helper.hpp"

// my files
// #include "./subdir/scenario1/multiplier.hpp"

#include <memory>
#include <vector>

namespace ns3 {

int
main(int argc, char* argv[])
{
  // setting default parameters for PointToPoint links and channels
  Config::SetDefault("ns3::PointToPointNetDevice::DataRate", StringValue("1Mbps"));
  Config::SetDefault("ns3::PointToPointChannel::Delay", StringValue("10ms"));
//   Config::SetDefault("ns3::DropTailQueue::MaxPackets", StringValue("20"));
  Config::SetDefault("ns3::QueueBase::MaxSize", StringValue("20p"));

  // Read optional command-line parameters (e.g., enable visualizer with ./waf --run=<> --visualize
  CommandLine cmd;
  cmd.Parse(argc, argv);

  // Creating nodes
  // NodeContainer nodes;
  // nodes.Create(5);

  // Connecting nodes using two links
  // PointToPointHelper p2p;
  // p2p.Install(nodes.Get(0), nodes.Get(3));
  // p2p.Install(nodes.Get(1), nodes.Get(3));
  // p2p.Install(nodes.Get(2), nodes.Get(3));
  // p2p.Install(nodes.Get(3), nodes.Get(4));

  AnnotatedTopologyReader topologyReader("", 25);
  topologyReader.SetFileName("/home/btp/ndnSIM/my-simulations/extensions/cache-demonstration.txt");
  topologyReader.Read();



  // Install NDN stack on all nodes
  ndn::StackHelper ndnHelper;
  // ndnHelper.SetDefaultRoutes(true);
  ndnHelper.InstallAll();

  // Choosing forwarding strategy
  ndn::StrategyChoiceHelper::InstallAll("/", "/localhost/nfd/strategy/best-route");

  // Installing global routing interface on all nodes
  ndn::GlobalRoutingHelper ndnGlobalRoutingHelper;
  ndnGlobalRoutingHelper.InstallAll();

  // Installing applications

  // Consumer
  ndn::AppHelper consumerHelper("ns3::ndn::ConsumerC1");
  // Consumer will request /prefix/0, /prefix/1, ...
  consumerHelper.SetPrefix("/prefix");
  consumerHelper.SetAttribute("Frequency", StringValue("10")); // 10 interests a second
  consumerHelper.Install(Names::Find<Node>("Node0"));
  consumerHelper.Install(Names::Find<Node>("Node1"));
  consumerHelper.Install(Names::Find<Node>("Node2"));

  // Producer
  ndn::AppHelper producerHelper("ns3::ndn::Producer");
  // Producer will reply to all requests starting with /prefix
  producerHelper.SetPrefix("/prefix");
  producerHelper.SetAttribute("PayloadSize", StringValue("1024"));
  producerHelper.SetAttribute("Freshness", TimeValue(Seconds(1)));
  producerHelper.Install(Names::Find<Node>("Node4")); // last node

  ndnGlobalRoutingHelper.AddOrigins("/prefix", Names::Find<Node>("Node4"));

  // Calculate and install FIBs
  ndn::GlobalRoutingHelper::CalculateRoutes();

  Simulator::Stop(Seconds(20.0));

  Simulator::Run();
  Simulator::Destroy();

  return 0;
}

} // namespace ns3

int
main(int argc, char* argv[])
{
  return ns3::main(argc, argv);
}