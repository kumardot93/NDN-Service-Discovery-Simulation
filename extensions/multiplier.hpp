#include "ns3/ndnSIM/apps/ndn-producer.hpp"

namespace ns3{
    namespace ndn{
        class Multiplier: public ns3::ndn::Producer{
          public:
          static TypeId GetTypeId();

            void OnInterest(shared_ptr<const Interest> interest);
            private:
                Name m_prefix;
                Name m_postfix;
                uint32_t m_virtualPayloadSize;
                Time m_freshness;

                uint32_t m_signature;
                Name m_keyLocator;
        };
    }
}

