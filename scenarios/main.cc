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

// for LinkStatusControl::FailLinks and LinkStatusControl::UpLinks
#include "ns3/ndnSIM/helper/ndn-link-control-helper.hpp"

// my files
// #include "./subdir/scenario1/multiplier.hpp"

#include <memory>
#include <vector>

namespace ns3
{

  int main(int argc, char *argv[])
  {
    // setting default parameters for PointToPoint links and channels
    Config::SetDefault("ns3::PointToPointNetDevice::DataRate", StringValue("1Mbps"));
    Config::SetDefault("ns3::PointToPointChannel::Delay", StringValue("10ms"));
    //   Config::SetDefault("ns3::DropTailQueue::MaxPackets", StringValue("20"));
    Config::SetDefault("ns3::QueueBase::MaxSize", StringValue("20p"));

    // Read optional command-line parameters (e.g., enable visualizer with ./waf --run=<> --visualize
    CommandLine cmd;
    cmd.Parse(argc, argv);

    AnnotatedTopologyReader topologyReader("", 25);
    // topologyReader.SetFileName("/home/ndn-experiment/NDN-Service-Discovery-Simulation/extensions/single-service-demonstration.txt");
    topologyReader.SetFileName("extensions/single-service-demonstration.txt");
    topologyReader.Read();

    // Creating nodes
    // NodeContainer nodes;
    // nodes.Create(3);

    // Connecting nodes using two links
    // PointToPointHelper p2p;
    // p2p.Install(nodes.Get(0), nodes.Get(1));
    // p2p.Install(nodes.Get(1), nodes.Get(2));

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

    // Consumer, will be defined later
    // ndn::AppHelper consumerHelper("ns3::ndn::ConsumerCbr");
    // // Consumer will request /prefix/0, /prefix/1, ...
    // consumerHelper.SetPrefix("/serviceA1");
    // consumerHelper.SetAttribute("Frequency", StringValue("2")); // 10 interests a second
    // consumerHelper.Install(Names::Find<Node>("C1"));

    ndn::AppHelper managerHelper("ns3::ndn::Manager");
    managerHelper.SetAttribute("Frequency", StringValue("10")); // 10 interests a second
    managerHelper.Install(Names::Find<Node>("Manager"));

    std::vector<string> producer_names{
        "serviceA1",
        "serviceA2",
        "serviceA3",
        "serviceB1",
        "serviceB2",
        "serviceC1",
        "serviceC2",
        "serviceC3",
        "serviceC4",
    };
    std::vector<string> consumer_names{
        "C1",
        "C2",
        "C3",
        "C4",
        "C5",
        "C6",
        "C7",
        "C8",
        "C9",
        "C10",
        "C11",
        "C12",
    };
    std::vector<string> service_map_to_consumer{
        "serviceA", "serviceA", "serviceB", "serviceC", "serviceC", "serviceA", "serviceA", "serviceB", "serviceC", "serviceC", "serviceA", "serviceA"};
    for (int i = 0; i < producer_names.size(); i++)
    {
      // Producer
      ndn::AppHelper producerHelper("ns3::ndn::Producer");
      producerHelper.SetPrefix("/" + producer_names[i]);
      producerHelper.SetAttribute("PayloadSize", StringValue("1024"));
      producerHelper.SetAttribute("Freshness", TimeValue(Seconds(1)));
      producerHelper.Install(Names::Find<Node>(producer_names[i]));
      ndnGlobalRoutingHelper.AddOrigins("/" + producer_names[i], Names::Find<Node>(producer_names[i]));
    }

    for (int i = 0; i < consumer_names.size(); i++)
    {
      // Producer
      ndn::AppHelper consumerHelper("ns3::ndn::MainConsumer");
      consumerHelper.SetPrefix(service_map_to_consumer[i]);
      consumerHelper.SetAttribute("Frequency", StringValue("2"));
      consumerHelper.SetAttribute("Randomize", StringValue("uniform"));
      consumerHelper.Install(Names::Find<Node>(consumer_names[i]));
    }

    ndnGlobalRoutingHelper.AddOrigins("/Manager", Names::Find<Node>("Manager"));

    Simulator::Schedule(Seconds(100.0), ndn::LinkControlHelper::FailLink, Names::Find<Node>("Manager"), Names::Find<Node>("serviceB1"));
    Simulator::Schedule(Seconds(200.0), ndn::LinkControlHelper::UpLink, Names::Find<Node>("Manager"), Names::Find<Node>("serviceB1"));

    Simulator::Schedule(Seconds(50.0), ndn::LinkControlHelper::FailLink, Names::Find<Node>("Manager"), Names::Find<Node>("serviceA1"));
    Simulator::Schedule(Seconds(150.0), ndn::LinkControlHelper::UpLink, Names::Find<Node>("Manager"), Names::Find<Node>("serviceA1"));

    Simulator::Schedule(Seconds(50.0), ndn::LinkControlHelper::FailLink, Names::Find<Node>("Manager"), Names::Find<Node>("serviceA2"));
    Simulator::Schedule(Seconds(150.0), ndn::LinkControlHelper::UpLink, Names::Find<Node>("Manager"), Names::Find<Node>("serviceA2"));

    Simulator::Schedule(Seconds(50.0), ndn::LinkControlHelper::FailLink, Names::Find<Node>("Manager"), Names::Find<Node>("serviceC1"));
    Simulator::Schedule(Seconds(150.0), ndn::LinkControlHelper::UpLink, Names::Find<Node>("Manager"), Names::Find<Node>("serviceC1"));

    Simulator::Schedule(Seconds(100.0), ndn::LinkControlHelper::FailLink, Names::Find<Node>("Manager"), Names::Find<Node>("serviceC2"));
    Simulator::Schedule(Seconds(250.0), ndn::LinkControlHelper::UpLink, Names::Find<Node>("Manager"), Names::Find<Node>("serviceC2"));

    // Calculate and install FIBs
    ndn::GlobalRoutingHelper::CalculateRoutes();

    ndn::L3RateTracer::InstallAll("results/rate-trace.txt", Seconds(0.5));
    L2RateTracer::InstallAll("results/drop-trace.txt", Seconds(0.5));

    Simulator::Stop(Seconds(300.0));

    Simulator::Run();
    Simulator::Destroy();

    return 0;
  }

} // namespace ns3

int main(int argc, char *argv[])
{
  return ns3::main(argc, argv);
}