#include "manager.hpp"
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
#include<vector>
#include "helper/ndn-fib-helper.hpp"

NS_LOG_COMPONENT_DEFINE("ndn.Manager");

namespace ns3 {
namespace ndn {

NS_OBJECT_ENSURE_REGISTERED(Manager);

TypeId
  Manager::GetTypeId(void)
{
  static TypeId tid =
      TypeId("ns3::ndn::Manager")
          .SetGroupName("Ndn")
          .SetParent<ConsumerCbr>()
          .AddConstructor<Manager>()
          .AddAttribute(
            "Postfix",
            "Postfix that is added to the output data (e.g., for adding producer-uniqueness)",
            StringValue("/"), MakeNameAccessor(&Manager::m_postfix), MakeNameChecker())
          .AddAttribute("PayloadSize", "Virtual payload size for Content packets", UintegerValue(1024),
                        MakeUintegerAccessor(&Manager::m_virtualPayloadSize),
                        MakeUintegerChecker<uint32_t>())
          .AddAttribute("Freshness", "Freshness of data packets, if 0, then unlimited freshness",
                        TimeValue(Seconds(0)), MakeTimeAccessor(&Manager::m_freshness),
                        MakeTimeChecker())
          .AddAttribute(
            "Signature",
            "Fake signature, 0 valid signature (default), other values application-specific",
            UintegerValue(0), MakeUintegerAccessor(&Manager::m_signature),
            MakeUintegerChecker<uint32_t>())
          .AddAttribute("KeyLocator",
                        "Name to be used for key locator.  If root, then key locator is not used",
                        NameValue(), MakeNameAccessor(&Manager::m_keyLocator), MakeNameChecker());

  return tid;
}

void
Manager::StartApplication()
{
  ConsumerCbr::StartApplication();

  FibHelper::AddRoute(GetNode(), m_prefix, m_face, 0);
}

void
Manager::SendPacket()
{
  m_interestName = Name(producer_names[p_index]);
  producer_active[p_index] = false;
  p_index = (p_index + 1) % 9;
  ConsumerCbr::SendPacket();
}

void
Manager::ScheduleNextPacket()
{
  // double mean = 8.0 * m_payloadSize / m_desiredRate.GetBitRate ();
  // std::cout << "next: " << Simulator::Now().ToDouble(Time::S) + mean << "s\n";

  if (m_firstTime) {
    m_sendEvent = Simulator::Schedule(Seconds(0.0), &Manager::SendPacket, this);
    m_firstTime = false;
  }
  else if (!m_sendEvent.IsRunning())
    m_sendEvent = Simulator::Schedule(Seconds(9.0 / m_frequency), &Manager::SendPacket, this);


}

void Manager::OnData(shared_ptr<const Data> contentObject){
  Consumer::OnData(contentObject);

  std::stringstream ss;
  ss << contentObject->getName().at(0);
  std::string node_name;
  ss >> node_name;
  node_name = "/" + node_name;
  int i;
  for (i = 0; i < producer_names.size();i++){
    if(node_name.compare(producer_names[i])==0){
      producer_active[i] = true;
    }
  }
}


void
Manager::OnInterest(shared_ptr<const Interest> interest)
{
  App::OnInterest(interest); // tracing inside

  NS_LOG_FUNCTION(this << interest);

  if (!m_active)
    return;

  Name dataName(interest->getName());
  // dataName.append(m_postfix);
  // dataName.appendVersion();
  // interest->wireDecode();
  std::string service_name((const char*)interest->getApplicationParameters().value());
  service_name = service_name.substr(0, service_name.find(' '));
  // std::cout << "service_name: " << service_name << "."
  //           << "\n";

  std::vector<std::string> temp;
  for(int i=0;i<producer_names.size(); i++){
    if(producer_names[i].find(service_name)==0 && producer_active[i]){
      temp.push_back(producer_names[i]);
    }
  }

  if(temp.size()==0){
    return;
  }

  std::string response = temp[rand()%temp.size()];
  temp.clear();
  // std::cout << "response_name: " << response << "."
  //           << "\n";

  auto data = make_shared<Data>();
  data->setName(dataName);
  data->setFreshnessPeriod(::ndn::time::milliseconds(m_freshness.GetMilliSeconds()));



  data->setContent(make_shared< ::ndn::Buffer>(response.c_str(), response.length()));

  Signature signature;
  SignatureInfo signatureInfo(static_cast< ::ndn::tlv::SignatureTypeValue>(255));

  if (m_keyLocator.size() > 0) {
    signatureInfo.setKeyLocator(m_keyLocator);
  }

  signature.setInfo(signatureInfo);
  signature.setValue(::ndn::makeNonNegativeIntegerBlock(::ndn::tlv::SignatureValue, m_signature));

  data->setSignature(signature);

  NS_LOG_INFO("node(" << GetNode()->GetId() << ") responding with Data: " << data->getName());

  // to create real wire encoding
  data->wireEncode();

  m_transmittedDatas(data, this, m_face);
  m_appLink->onReceiveData(*data);
}


} // namespace ndn
} // namespace ns3
