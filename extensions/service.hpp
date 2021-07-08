#include "ns3/ndnSIM/apps/ndn-consumer-cbr.hpp"
#include <string>
#include <vector>

namespace ns3
{
    namespace ndn
    {

        class Service : public ConsumerCbr
        {

        public:
            static TypeId
            GetTypeId();

            virtual void
            StartApplication(); // Called at time specified by Start

            // void SendPacket();

            // virtual void
            // OnData(shared_ptr<const Data> contentObject);

            virtual void
            OnInterest(shared_ptr<const Interest> interest);

            void ScheduleNextPacket();

            // std::string servicesPathList[2] = {"/discover-service",
            //                                    "/register-service"};

        private:
            Name m_prefix;
            Name m_postfix;
            uint32_t m_virtualPayloadSize;
            Time m_freshness;

            uint32_t m_signature;
            Name m_keyLocator;
            std::string keywords;
        };
    }
}