#include "root-router.hpp"
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

NS_LOG_COMPONENT_DEFINE("ndn.RootRouter");

namespace ns3
{
    namespace ndn
    {

        NS_OBJECT_ENSURE_REGISTERED(RootRouter);

        TypeId
        RootRouter::GetTypeId(void)
        {
            static TypeId tid =
                TypeId("ns3::ndn::RootRouter")
                    .SetGroupName("Ndn")
                    .SetParent<ConsumerCbr>()
                    .AddConstructor<RootRouter>()
                    .AddAttribute(
                        "Postfix",
                        "Postfix that is added to the output data (e.g., for adding producer-uniqueness)",
                        StringValue("/"), MakeNameAccessor(&RootRouter::m_postfix), MakeNameChecker())
                    .AddAttribute("PayloadSize", "Virtual payload size for Content packets", UintegerValue(1024),
                                  MakeUintegerAccessor(&RootRouter::m_virtualPayloadSize),
                                  MakeUintegerChecker<uint32_t>())
                    .AddAttribute("Freshness", "Freshness of data packets, if 0, then unlimited freshness",
                                  TimeValue(Seconds(0)), MakeTimeAccessor(&RootRouter::m_freshness),
                                  MakeTimeChecker())
                    .AddAttribute(
                        "Signature",
                        "Fake signature, 0 valid signature (default), other values application-specific",
                        UintegerValue(0), MakeUintegerAccessor(&RootRouter::m_signature),
                        MakeUintegerChecker<uint32_t>())
                    .AddAttribute("KeyLocator",
                                  "Name to be used for key locator.  If root, then key locator is not used",
                                  NameValue(), MakeNameAccessor(&RootRouter::m_keyLocator), MakeNameChecker());

            return tid;
        }

        void
        RootRouter::StartApplication()
        {
            ConsumerCbr::StartApplication();

            FibHelper::AddRoute(GetNode(), m_prefix, m_face, 0);
        }

        void
        RootRouter::SendPacket()
        {
            m_interestName = Name(CHILDREN_DIRECTORY[ping_index].first);
            ping_index = (ping_index + 1) % CHILDREN_DIRECTORY.size();
            ConsumerCbr::SendPacket();
        }

        void
        RootRouter::ScheduleNextPacket()
        {
            if (CHILDREN_DIRECTORY.size() == 0)
                return;

            if (m_firstTime)
            {
                m_sendEvent = Simulator::Schedule(Seconds(0.0), &RootRouter::SendPacket, this);
                m_firstTime = false;
            }
            else if (!m_sendEvent.IsRunning())
                m_sendEvent = Simulator::Schedule(Seconds(30 / CHILDREN_DIRECTORY.size()), &RootRouter::SendPacket, this);
        }

        void RootRouter::OnData(shared_ptr<const Data> contentObject)
        {
            Consumer::OnData(contentObject);

            std::stringstream ss;
            ss << contentObject->getName().at(0);
            std::string node_name;
            ss >> node_name;
            node_name = "/" + node_name;
            int i;
            for (i = 0; i < CHILDREN_DIRECTORY.size(); i++)
            {
                if (node_name.compare(CHILDREN_DIRECTORY[i].first) == 0)
                {
                    CHILDREN_DIRECTORY[i].second = std::time(0) + 120; // expiry time set to current time + 2 minutes
                }
            }
            if (CHILDREN_DIRECTORY.size() == 1 && !m_sendEvent.IsRunning())
                this->ScheduleNextPacket();
        }

        void
        RootRouter::OnInterest(shared_ptr<const Interest> interest)
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
                for (int i = 0; i < CHILDREN_DIRECTORY.size(); i++)
                {
                    if (CHILDREN_DIRECTORY[i].second > std::time(0))
                    {
                        resposne += CHILDREN_DIRECTORY[i].first + " ";
                    }
                }
            }

            else if (uri.find(m_prefix.toUri() + servicesPathList[1]) == 0)
            { // register contact
                std::string incoming_data((const char *)interest->getApplicationParameters().value());
                local_name = service_name.substr(0, service_name.find(' '));
                bool alreadyPresent = false;
                for (int i = 0; i < CHILDREN_DIRECTORY.size(); i++)
                {
                    if (local_name.compare(CHILDREN_DIRECTORY[i].first) == 0)
                    {
                        alreadyPresent = true;
                        break;
                    }
                }
                if (!alreadyPresent) // new node in local network
                {
                    CHILDREN_DIRECTIRY.push_back(std::make_pair(local_name, std::time(0) + 120));
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
