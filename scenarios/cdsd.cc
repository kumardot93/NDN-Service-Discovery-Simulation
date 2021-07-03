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
        // Config::SetDefault("ns3::PointToPointNetDevice::DataRate", StringValue("1Mbps"));
        // Config::SetDefault("ns3::PointToPointChannel::Delay", StringValue("10ms"));
        //   Config::SetDefault("ns3::DropTailQueue::MaxPackets", StringValue("20"));
        // Config::SetDefault("ns3::QueueBase::MaxSize", StringValue("20p"));

        // Read optional command-line parameters (e.g., enable visualizer with ./waf --run=<> --visualize
        CommandLine cmd;
        cmd.Parse(argc, argv);

        AnnotatedTopologyReader topologyReader("", 25);
        // topologyReader.SetFileName("/home/ndn-experiment/NDN-Service-Discovery-Simulation/extensions/single-service-demonstration.txt");
        topologyReader.SetFileName("extensions/ndcs.txt");
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

        std::vector<string> root_router_names{
            "root-router1",
        };

        for (int i = 0; i < root_router_names.size(); i++)
        {
            ndn::AppHelper rootRouterHelper("ns3::ndn::RootRouter");
            rootRouterHelper.Install(Names::Find<Node>(root_router_names[i]));
            ndnGlobalRoutingHelper.AddOrigins("/root-router", Names::Find<Node>(root_router_names[i]));
        }

        std::vector<string> consumer_names{
            "C1",
            "C2",
        };

        for (int i = 0; i < consumer_names.size(); i++)
        {
            ndn::AppHelper consumerHelper("ns3::ndn::ConsumerCbr");
            consumerHelper.SetPrefix("/root-router/register-contact");
            consumerHelper.SetAttribute("Frequency", StringValue("2"));
            consumerHelper.SetAttribute("Randomize", StringValue("uniform"));
            consumerHelper.Install(Names::Find<Node>(consumer_names[i]));
        }

        // Calculate and install FIBs
        ndn::GlobalRoutingHelper::CalculateRoutes();

        ndn::L3RateTracer::InstallAll("results/rate-trace.txt", Seconds(0.5));
        L2RateTracer::InstallAll("results/drop-trace.txt", Seconds(0.5));

        Simulator::Stop(Seconds(20.0));

        Simulator::Run();
        Simulator::Destroy();

        return 0;
    }

} // namespace ns3

int main(int argc, char *argv[])
{
    return ns3::main(argc, argv);
}