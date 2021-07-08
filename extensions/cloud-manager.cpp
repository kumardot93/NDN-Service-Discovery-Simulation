#include "cloud-manager.hpp"
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
#include <vector>
#include "helper/ndn-fib-helper.hpp"
#include <ctime>
#include <queue>

NS_LOG_COMPONENT_DEFINE("ndn.CloudManager");

// trim from start (in place)
static inline void ltrim(std::string &s)
{
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch)
                                    { return !std::isspace(ch); }));
}

// trim from end (in place)
static inline void rtrim(std::string &s)
{
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch)
                         { return !std::isspace(ch); })
                .base(),
            s.end());
}

// trim from both ends (in place)
static inline void trim(std::string &s)
{
    ltrim(s);
    rtrim(s);
}

namespace ns3
{
    namespace ndn
    {

        NS_OBJECT_ENSURE_REGISTERED(CloudManager);

        TypeId
        CloudManager::GetTypeId(void)
        {
            static TypeId tid =
                TypeId("ns3::ndn::CloudManager")
                    .SetGroupName("Ndn")
                    .SetParent<ConsumerCbr>()
                    .AddConstructor<CloudManager>()
                    .AddAttribute(
                        "Postfix",
                        "Postfix that is added to the output data (e.g., for adding producer-uniqueness)",
                        StringValue("/"), MakeNameAccessor(&CloudManager::m_postfix), MakeNameChecker())
                    .AddAttribute("PayloadSize", "Virtual payload size for Content packets", UintegerValue(1024),
                                  MakeUintegerAccessor(&CloudManager::m_virtualPayloadSize),
                                  MakeUintegerChecker<uint32_t>())
                    .AddAttribute("Freshness", "Freshness of data packets, if 0, then unlimited freshness",
                                  TimeValue(Seconds(0)), MakeTimeAccessor(&CloudManager::m_freshness),
                                  MakeTimeChecker())
                    .AddAttribute(
                        "Signature",
                        "Fake signature, 0 valid signature (default), other values application-specific",
                        UintegerValue(0), MakeUintegerAccessor(&CloudManager::m_signature),
                        MakeUintegerChecker<uint32_t>())
                    .AddAttribute("KeyLocator",
                                  "Name to be used for key locator.  If root, then key locator is not used",
                                  NameValue(), MakeNameAccessor(&CloudManager::m_keyLocator), MakeNameChecker());

            return tid;
        }

        void
        CloudManager::StartApplication()
        {
            ConsumerCbr::StartApplication();

            FibHelper::AddRoute(GetNode(), m_prefix, m_face, 0);
        }

        void
        CloudManager::SendPacket()
        {
            m_interestName = Name(SERVICES_DIRECTORY[ping_index].first);
            ping_index = (ping_index + 1) % SERVICES_DIRECTORY.size();
            ConsumerCbr::SendPacket();
        }

        void
        CloudManager::ScheduleNextPacket()
        {
            if (SERVICES_DIRECTORY.size() == 0)
                return;

            if (m_firstTime)
            {
                m_sendEvent = Simulator::Schedule(Seconds(0.0), &CloudManager::SendPacket, this);
                m_firstTime = false;
            }
            else if (!m_sendEvent.IsRunning())
                m_sendEvent = Simulator::Schedule(Seconds(60 / SERVICES_DIRECTORY.size()), &CloudManager::SendPacket, this);
        }

        void CloudManager::OnData(shared_ptr<const Data> contentObject)
        {
            Consumer::OnData(contentObject);

            std::stringstream ss;
            ss << contentObject->getName().at(0);
            std::string node_name;
            ss >> node_name;
            node_name = "/" + node_name;
            int i;
            for (i = 0; i < SERVICES_DIRECTORY.size(); i++)
            {
                if (node_name.compare(SERVICES_DIRECTORY[i].first) == 0)
                {
                    SERVICES_DIRECTORY[i].second.timeout = std::time(0) + 120; // expiry time set to current time + 2 minutes
                }
            }
            if (SERVICES_DIRECTORY.size() == 1 && !m_sendEvent.IsRunning())
                this->ScheduleNextPacket();
        }

        void
        CloudManager::OnInterest(shared_ptr<const Interest> interest)
        {

            App::OnInterest(interest); // tracing inside

            NS_LOG_FUNCTION(this << interest);

            if (!m_active)
                return;

            Name dataName(interest->getName());

            std::string uri = dataName.toUri();

            std::string response = "";

            if (uri.find(m_prefix.toUri() + servicesPathList[0]) == 0)
            {
                std::prioriy_queue<std::pair<float, int>, std::vector<std::pair<float, int> >, std::greater<std::pair<float, int> > > services_queue; // score, index

                std::string incoming_data((const char *)interest->getApplicationParameters().value());
                trim(incoming_data);
                std::vector<std::string> keyword_list;
                int start_index = 0;
                int pos = 0;
                int total_size = 0;
                while ((pos = incoming_data.find(',')) != std::string::npos)
                {
                    keyword_list.push_back(incoming_data.substr(start_index, pos));
                    start_index = pos + 1;
                    total_size += keyword_list.back().size();
                }
                if (pos != incoming_data.length())
                {
                    keyword_list.push_back(incoming_data.substr(start_index, incoming_data.length()));
                    total_size += keyword_list.back().size();
                }
                for (int i = 0; i < SERVICES_DIRECTORY.size(); i++)
                {
                    if (SERVICES_DIRECTORY[i].second.timeout <= std::time(0))
                        continue;
                    int score = 0;
                    for (auto &incoming_keyword : keyword_list)
                    {
                        for (auto service_keyword : SERVICES_DIRECTORY[i].second.keywords)
                        {
                            if (incoming_keyword.find(service_keyword) != std::string::npos)
                            {
                                score += service_keyword.length();
                            }
                            if (service_keyword.find(incoming_keyword) != std::string::npos)
                            {
                                score += incoming_keyword.length();
                            }
                        }
                    }
                    float normalized_score = ((float)score) / total_size;
                    services_queue.push(std::make_pair(normalized_score, i));
                }
                int max_service_in_response = 3;
                while (prioriy_queue.size() != 0 && max_service_in_response > 0)
                {
                    service_index = services_queue.top().second;
                    response += SERVICES_DIRECTORY[service_index].first + " ";
                    services_queue.pop();
                    max_service_in_response--;
                }
            }

            else if (uri.find(m_prefix.toUri() + servicesPathList[1]) == 0)
            { // register contact
                std::string incoming_data((const char *)interest->getApplicationParameters().value());
                local_name = service_name.substr(0, service_name.find(' '));
                bool alreadyPresent = false;
                for (int i = 0; i < SERVICES_DIRECTORY.size(); i++)
                {
                    if (local_name.compare(SERVICES_DIRECTORY[i].first) == 0)
                    {
                        alreadyPresent = true;
                        break;
                    }
                }
                if (!alreadyPresent) // new node in local network
                {
                    ServiceInfo serviceDescription;
                    serviceDescription.active = true;
                    serviceDescription.timeout = std::time(0) + 120;
                    int pos = 0;
                    int start_index = 0;
                    while ((pos = incoming_data.find(',')) != std::string::npos)
                    {
                        serviceDescrition.keywords.push_back(incoming_data.substr(start_index, pos));
                        start_index = pos + 1;
                    }
                    if (pos != incoming_data.length())
                    {
                        serviceDescrition.keywords.push_back(incoming_data.substr(start_index, incoming_data.length()));
                    }
                    auto newService = std::make_pair(local_name, );
                    SERVICES_DIRECTORY.push_back(newService);
                }
            }

            return;

            auto data = make_shared<Data>();
            data->setName(dataName);
            data->setFreshnessPeriod(::ndn::time::milliseconds(m_freshness.GetMilliSeconds()));

            data->setContent(make_shared< ::ndn::Buffer>(response.c_str(), response.length()));

            Signature signature;
            SignatureInfo signatureInfo(static_cast< ::ndn::tlv::SignatureTypeValue>(255));

            if (m_keyLocator.size() > 0)
            {
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
