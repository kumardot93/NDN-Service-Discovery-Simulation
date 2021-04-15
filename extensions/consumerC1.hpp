#include "ns3/ndnSIM/apps/ndn-consumer-cbr.hpp"

namespace ns3 {
namespace ndn {


    class ConsumerC1 : public ConsumerCbr {

        public:

        // ConsumerC1();

        static TypeId
        GetTypeId();

        void ScheduleNextPacket();
    };
}
}