#include "ns3/ndnSIM/apps/ndn-consumer-cbr.hpp"

namespace ns3 {
namespace ndn {


    class MultiplierConsumer : public ConsumerCbr {

        public:

        static TypeId
        GetTypeId();

        void SendPacket();

        void ScheduleNextPacket();
    };
}
}