#include "multiplier.hpp"

#include "ns3/log.h"
#include "ns3/string.h"
#include "ns3/uinteger.h"
#include "ns3/packet.h"
#include "ns3/simulator.h"

#include "helper/ndn-fib-helper.hpp"

#include<string.h>
#include<vector>
#include<string>

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

        void Multiplier::multiply(const uint8_t *buffer, uint8_t *res){
            std::vector<int> num_array;
            char local[30];
            strcpy(local, (const char*)buffer);
            char *token = strtok(local, " ");
            while(token!=NULL){
                num_array.push_back(atoi(token));
                token = strtok(NULL, " ");
            }
            int result = 1;
            for(int i=0; i<num_array.size();i++){
                result = result * num_array[i];
            }
            strcpy((char*)res, std::to_string(result).c_str());
            num_array.clear();
            free(local);
        }

        void ns3::ndn::Multiplier::OnInterest(shared_ptr<const Interest> interest)
        {
            // std::cout<<"multiplier servir working"<<"\n";
            App::OnInterest(interest); // tracing inside
            // cout<<interest->toUri()<<"\n";
            // NS_LOG_FUNCTION(this << interest);

            if (!m_active)
                return;

            const uint8_t *parameterData =  interest->getApplicationParameters().value();
            std::cout << "received parameter data: " << parameterData << "\n";

            uint8_t res_buffer[30];
            multiply(parameterData, res_buffer);
            std::cout << "res buffer: " << res_buffer << "\n";

            Name dataName(interest->getName());
            // dataName.append(m_postfix);
            // dataName.appendVersion();
            std::cout << "intest->getName: " << interest->getName()<<"\n";

            auto data = make_shared<Data>();
            data->setName(dataName);
            data->setFreshnessPeriod(::ndn::time::milliseconds(m_freshness.GetMilliSeconds()));

            data->setContent(make_shared< ::ndn::Buffer>(m_virtualPayloadSize));

            // std::vector <u_int8_t> v = {11,2,30};
            // std::string str = "abcd";
            data->setContent(make_shared< ::ndn::Buffer>(res_buffer, strlen((char*)res_buffer)));
            // std::cout<<data->getContent()<<"\n";
            // free(res_buffer);
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