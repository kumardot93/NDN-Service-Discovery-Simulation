#include "multiplier.hpp"

#include "ns3/log.h"
#include "ns3/string.h"
#include "ns3/uinteger.h"
#include "ns3/packet.h"
#include "ns3/simulator.h"

#include "helper/ndn-fib-helper.hpp"

// my files
// #include "./subdir/scenario1/multiplier.hpp"

#include <memory>
#include <vector>
#include <string>

NS_LOG_COMPONENT_DEFINE("ndn.Multiplier");

namespace ns3{
    namespace ndn{
        NS_OBJECT_ENSURE_REGISTERED(Multiplier);
        TypeId ns3::ndn::Multiplier::GetTypeId(){
            static TypeId tid = TypeId("ns3::ndn::Multiplier")
                                    .SetParent<ndn::Producer>()
                                    .AddConstructor<Multiplier>();
            return tid;
        }

        void ns3::ndn::Multiplier::OnInterest(shared_ptr<const Interest> interest)
        {
            std::cout<<"multiplier servir working"<<"\n";
            App::OnInterest(interest); // tracing inside
            // cout<<interest->toUri()<<"\n";
            // NS_LOG_FUNCTION(this << interest);

            if (!m_active)
                return;

            Name dataName(interest->getName());
            // dataName.append(m_postfix);
            // dataName.appendVersion();
            std::cout << "intest->getName: " << interest->getName()<<"\n";

            auto data = make_shared<Data>();
            data->setName(dataName);
            data->setFreshnessPeriod(::ndn::time::milliseconds(m_freshness.GetMilliSeconds()));

            // data->setContent(make_shared< ::ndn::Buffer>(m_virtualPayloadSize));

            std::vector <u_int8_t> v = {11,2,30};
            std::string str = "abcd";
            data->setContent(make_shared< ::ndn::Buffer>(v.begin(), v.end()));
            // std::cout<<data->getContent()<<"\n";

            Signature signature;
            SignatureInfo signatureInfo(static_cast< ::ndn::tlv::SignatureTypeValue>(255));

            if (m_keyLocator.size() > 0) {
                signatureInfo.setKeyLocator(m_keyLocator);
            }

            signature.setInfo(signatureInfo);
            signature.setValue(::ndn::makeNonNegativeIntegerBlock(::ndn::tlv::SignatureValue, m_signature));

            data->setSignature(signature);

            // NS_LOG_INFO("node(" << GetNode()->GetId() << ") responding with Data: " << data->getName());

            // to create real wire encoding
            data->wireEncode();

            m_transmittedDatas(data, this, m_face);
            m_appLink->onReceiveData(*data);
        }
      
    }
}