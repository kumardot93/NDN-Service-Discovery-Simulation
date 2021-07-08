#include "service.hpp"
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

NS_LOG_COMPONENT_DEFINE("ndn.Service");

namespace ns3
{
    namespace ndn
    {

        NS_OBJECT_ENSURE_REGISTERED(Service);

        TypeId
        Service::GetTypeId(void)
        {
            static TypeId tid =
                TypeId("ns3::ndn::Service")
                    .SetGroupName("Ndn")
                    .SetParent<ConsumerCbr>()
                    .AddConstructor<Service>()
                    .AddAttribute(
                        "Postfix",
                        "Postfix that is added to the output data (e.g., for adding producer-uniqueness)",
                        StringValue("/"), MakeNameAccessor(&Service::m_postfix), MakeNameChecker())
                    .AddAttribute("PayloadSize", "Virtual payload size for Content packets", UintegerValue(1024),
                                  MakeUintegerAccessor(&Service::m_virtualPayloadSize),
                                  MakeUintegerChecker<uint32_t>())
                    .AddAttribute("Freshness", "Freshness of data packets, if 0, then unlimited freshness",
                                  TimeValue(Seconds(0)), MakeTimeAccessor(&Service::m_freshness),
                                  MakeTimeChecker())
                    .AddAttribute(
                        "Signature",
                        "Fake signature, 0 valid signature (default), other values application-specific",
                        UintegerValue(0), MakeUintegerAccessor(&Service::m_signature),
                        MakeUintegerChecker<uint32_t>())
                    .AddAttribute("KeyLocator",
                                  "Name to be used for key locator.  If root, then key locator is not used",
                                  NameValue(), MakeNameAccessor(&Service::m_keyLocator), MakeNameChecker())
                    .AddAttribute("keywords",
                                  "Keywords that describes this service",
                                  "");

            return tid;
        }

        void
        Service::StartApplication()
        {
            ConsumerCbr::StartApplication();

            FibHelper::AddRoute(GetNode(), m_prefix, m_face, 0);
            this->ServiceRegistration();
        }

        void Service::ServiceRegistration()
        {
            // m_interestName
            if (!m_active)
                return;

            uint32_t seq = std::numeric_limits<uint32_t>::max(); // invalid

            while (m_retxSeqs.size())
            {
                seq = *m_retxSeqs.begin();
                m_retxSeqs.erase(m_retxSeqs.begin());
                break;
            }

            if (seq == std::numeric_limits<uint32_t>::max())
            {
                if (m_seqMax != std::numeric_limits<uint32_t>::max())
                {
                    if (m_seq >= m_seqMax)
                    {
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

            std::string buffer_str(keywords);
            int padding_len = 32 - buffer_str.length() % 32;
            buffer_str = buffer_str + std::string(padding_len, ' ');
            uint8_t *b = (uint8_t *)buffer_str.c_str();
            interest->setApplicationParameters(b, buffer_str.size());

            WillSendOutInterest(seq);

            m_transmittedInterests(interest, this, m_face);
            m_appLink->onReceiveInterest(*interest);

            Simulator::Schedule(Seconds(m_rand->GetValue(4, 10)), &MainConsumer::ServiceDiscovery, this);
        }

        void Service::OnData(shared_ptr<const Data> contentObject)
        {
            Consumer::OnData(contentObject);
            std::stringstream ss;
            std::string name;
            ss << contentObject->getName();
            ss >> name;
            if (name.find("/cloud-manager") == 0)
            {
                // char dest[15] = "";
                // memcpy(dest, (const char *)contentObject->getContent().value(), 10);
                // producer_name = Name((const char *)dest);
                // producer_outdated = false;
                // std::cout << "main Consumer produce name: " << producer_name << "\n";
                // MainConsumer::SendPacket();
            }
        }

        void
        Service::ScheduleNextPacket() {}

        void
        Service::OnInterest(shared_ptr<const Interest> interest)
        {

            App::OnInterest(interest); // tracing inside

            NS_LOG_FUNCTION(this << interest);

            if (!m_active)
                return;

            Name dataName(interest->getName());

            std::string response(m_virtualPayloadSize, 'x');

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
