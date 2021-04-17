#include "ns3/ndnSIM/apps/ndn-consumer-cbr.hpp"
#include<string>
#include<vector>

namespace ns3 {
namespace ndn {


    class Manager : public ConsumerCbr {

        public:
            std::vector<std::string> producer_names{"/serviceA1", "/serviceA2", "/serviceA3", "/serviceB1", "/serviceB2", "/serviceC1", "/serviceC2", "/serviceC3", "/serviceC4"};
            std::vector<std::string> producer_services{"serviceA", "serviceA", "serviceA", "serviceB", "serviceB", "serviceC", "serviceC", "serviceC", "serviceC"};
            std::vector<bool> producer_active = std::vector<bool>(9, true);

            int p_index = 0;

            static TypeId
            GetTypeId();

            virtual void
                StartApplication(); // Called at time specified by Start

            void SendPacket();

            virtual void
                OnData(shared_ptr<const Data> contentObject);

            virtual void
                OnInterest(shared_ptr<const Interest> interest);

            void ScheduleNextPacket();


        private:
            Name m_prefix = Name("/Manager");
            Name m_postfix;
            uint32_t m_virtualPayloadSize;
            Time m_freshness;

            uint32_t m_signature;
            Name m_keyLocator;
    };
}
}