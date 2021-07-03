#include "ns3/ndnSIM/apps/ndn-consumer-cbr.hpp"
#include <string>
#include <vector>

namespace ns3
{
    namespace ndn
    {

        class RootRouter : public ConsumerCbr
        {

        public:
            // childrent directory <name of node, active till timestamp>
            std::vector<std::pair<std::string, long int> > CHILDREN_DIRECTORY;

            int ping_index = 0;

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

            std::string servicesPathList[2] = {"/local-contacts",
                                               "/register-contact"};

        private:
            Name m_prefix = Name("/root-router");
            Name m_postfix;
            uint32_t m_virtualPayloadSize;
            Time m_freshness;

            uint32_t m_signature;
            Name m_keyLocator;
        };
    }
}