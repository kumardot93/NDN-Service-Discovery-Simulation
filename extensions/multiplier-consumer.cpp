#include "multiplier-consumer.hpp"
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
#include <iostream>
#include <string>

NS_LOG_COMPONENT_DEFINE("ndn.MultiplierConsumer");

namespace ns3 {
namespace ndn {

NS_OBJECT_ENSURE_REGISTERED(MultiplierConsumer);

TypeId
  MultiplierConsumer::GetTypeId(void)
{
  static TypeId tid =
      TypeId("ns3::ndn::MultiplierConsumer")
          .SetGroupName("Ndn")
          .SetParent<ConsumerCbr>()
          .AddConstructor<MultiplierConsumer>();

  return tid;
}

void
MultiplierConsumer::SendPacket()
{
  if (!m_active)
    return;

  NS_LOG_FUNCTION_NOARGS();

  uint32_t seq = std::numeric_limits<uint32_t>::max(); // invalid

  while (m_retxSeqs.size()) {
    seq = *m_retxSeqs.begin();
    m_retxSeqs.erase(m_retxSeqs.begin());
    break;
  }

  if (seq == std::numeric_limits<uint32_t>::max()) {
    if (m_seqMax != std::numeric_limits<uint32_t>::max()) {
      if (m_seq >= m_seqMax) {
        return; // we are totally done
      }
    }

    seq = m_seq++;
  }



  shared_ptr<Name> nameWithSequence = make_shared<Name>(m_interestName);
  nameWithSequence->appendParametersSha256DigestPlaceholder();
  nameWithSequence->appendSequenceNumber(seq);

  // shared_ptr<Interest> interest = make_shared<Interest> ();
  shared_ptr<Interest> interest = make_shared<Interest>();
  interest->setNonce(m_rand->GetValue(0, std::numeric_limits<uint32_t>::max()));
  interest->setName(*nameWithSequence);
  interest->setCanBePrefix(false);
  time::milliseconds interestLifeTime(m_interestLifeTime.GetMilliSeconds());
  interest->setInterestLifetime(interestLifeTime);

  int rand1 = m_rand->GetValue(0, 100), rand2 = m_rand->GetValue(0, 100);
  
  std::string buffer_str = std::to_string(rand1);
  buffer_str.append(" ");
  buffer_str.append(std::to_string(rand2));
  int padding_len = 32 - buffer_str.length() % 32;
  buffer_str = buffer_str + std::string(padding_len, ' ');
  uint8_t *b = (uint8_t *)buffer_str.c_str();
  interest->setApplicationParameters(b, buffer_str.size());
  std::cout << "application params set"
            << b<<"\n";

  // NS_LOG_INFO ("Requesting Interest: \n" << *interest);
  NS_LOG_INFO("> Interest for " << seq);

  WillSendOutInterest(seq);
  std::cout << "will send out intrest success"
            << "\n";

  m_transmittedInterests(interest, this, m_face);
  std::cout << "transmission done"
            << "\n";
  m_appLink->onReceiveInterest(*interest);

  ScheduleNextPacket();
}

void
MultiplierConsumer::ScheduleNextPacket()
{
  // double mean = 8.0 * m_payloadSize / m_desiredRate.GetBitRate ();
  // std::cout << "next: " << Simulator::Now().ToDouble(Time::S) + mean << "s\n";

  if (m_firstTime) {
    m_sendEvent = Simulator::Schedule(Seconds(0.0), &MultiplierConsumer::SendPacket, this);
    m_firstTime = false;
  }
  else if (!m_sendEvent.IsRunning())
    m_sendEvent = Simulator::Schedule((m_random == 0) ? Seconds(1.0 / m_frequency)
                                                      : Seconds(m_random->GetValue()),
                                      &MultiplierConsumer::SendPacket, this);
}



} // namespace ndn
} // namespace ns3
