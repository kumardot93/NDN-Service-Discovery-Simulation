#include "consumerC1.hpp"
#include "ns3/ptr.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/packet.h"
#include "ns3/callback.h"
#include "ns3/string.h"
#include "ns3/boolean.h"
#include "ns3/uinteger.h"
#include "ns3/integer.h"
#include "ns3/double.h"

NS_LOG_COMPONENT_DEFINE("ndn.ConsumerC1");

namespace ns3 {
namespace ndn {

NS_OBJECT_ENSURE_REGISTERED(ConsumerC1);

TypeId
ConsumerC1::GetTypeId(void)
{
  static TypeId tid =
      TypeId("ns3::ndn::ConsumerC1")
          .SetGroupName("Ndn")
          .SetParent<ConsumerCbr>()
          .AddConstructor<ConsumerC1>();

  return tid;
}


} // namespace ndn
} // namespace ns3
