#include "main-consumer.hpp"
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
#include <sstream>
#include <string>

NS_LOG_COMPONENT_DEFINE("ndn.MainConsumer");

namespace ns3 {
namespace ndn {

NS_OBJECT_ENSURE_REGISTERED(MainConsumer);

TypeId
MainConsumer::GetTypeId(void)
{
  static TypeId tid =
      TypeId("ns3::ndn::MainConsumer")
          .SetGroupName("Ndn")
          .SetParent<ConsumerCbr>()
          .AddConstructor<MainConsumer>();

  return tid;
}

void MainConsumer::ServiceDiscovery(){
  // m_interestName
  if (!m_active)
      return;


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

  Name managerName("/Manager");
  //
  shared_ptr<Name> nameWithSequence = make_shared<Name>(managerName);
  nameWithSequence->appendParametersSha256DigestPlaceholder();
  nameWithSequence->appendSequenceNumber(seq);
  //

  // shared_ptr<Interest> interest = make_shared<Interest> ();
  shared_ptr<Interest> interest = make_shared<Interest>();
  interest->setNonce(m_rand->GetValue(0, std::numeric_limits<uint32_t>::max()));
  interest->setName(*nameWithSequence);
  interest->setCanBePrefix(false);
  time::milliseconds interestLifeTime(m_interestLifeTime.GetMilliSeconds());
  interest->setInterestLifetime(interestLifeTime);

  std::string buffer_str(m_interestName.toUri());
  int padding_len = 32 - buffer_str.length() % 32;
  buffer_str = buffer_str + std::string(padding_len, ' ');
  uint8_t *b = (uint8_t *)buffer_str.c_str();
  interest->setApplicationParameters(b, buffer_str.size());

  WillSendOutInterest(seq);

  m_transmittedInterests(interest, this, m_face);
  m_appLink->onReceiveInterest(*interest);


  Simulator::Schedule(Seconds(m_rand->GetValue(4,10)), &MainConsumer::ServiceDiscovery, this);

}

void MainConsumer::ScheduleNextPacket(){

  if (m_firstTime) {
    m_sendEvent = Simulator::Schedule(Seconds(0.0), &MainConsumer::SendPacket, this);
    m_firstTime = false;
  }
  else if (!m_sendEvent.IsRunning())
    m_sendEvent = Simulator::Schedule((m_random == 0) ? Seconds(1.0 / m_frequency)
                                                      : Seconds(m_random->GetValue()),
                                      &MainConsumer::SendPacket, this);

}

void MainConsumer::OnData(shared_ptr<const Data> contentObject){
  Consumer::OnData(contentObject);
  std::stringstream ss;
  std::string name;
  ss << contentObject->getName();
  ss >> name;
  if (name.find("/Manager")==0)
  {
    char dest[15] = "";
    memcpy(dest, (const char *)contentObject->getContent().value(), 10);
    producer_name = Name((const char *)dest);
    producer_outdated = false;
    std::cout << "main Consumer produce name: " << producer_name << "\n";
    MainConsumer::SendPacket();
  }
}

void MainConsumer::SendPacket(){

  if (!m_active)
    return;

  if(producer_name.empty() || producer_outdated){
    MainConsumer::ServiceDiscovery();
    return;
  }

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

  //
  shared_ptr<Name> nameWithSequence = make_shared<Name>(producer_name);
  nameWithSequence->appendSequenceNumber(seq);
  //

  // shared_ptr<Interest> interest = make_shared<Interest> ();
  shared_ptr<Interest> interest = make_shared<Interest>();
  interest->setNonce(m_rand->GetValue(0, std::numeric_limits<uint32_t>::max()));
  interest->setName(*nameWithSequence);
  interest->setCanBePrefix(false);
  time::milliseconds interestLifeTime(m_interestLifeTime.GetMilliSeconds());
  interest->setInterestLifetime(interestLifeTime);

  // NS_LOG_INFO ("Requesting Interest: \n" << *interest);
  NS_LOG_INFO("> Interest for " << seq);

  WillSendOutInterest(seq);

  m_transmittedInterests(interest, this, m_face);
  m_appLink->onReceiveInterest(*interest);

  ScheduleNextPacket();
  if(producer_outdated && !m_sendEvent.IsRunning()){
    m_sendEvent = Simulator::Schedule(Seconds(10.0), &MainConsumer::ServiceDiscovery, this);
  }
}

} // namespace ndn
} // namespace ns3
