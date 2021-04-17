#include "ns3/ndnSIM/apps/ndn-consumer-cbr.hpp"

namespace ns3 {
namespace ndn {


    class MainConsumer : public ConsumerCbr {

        public:

        // ConsumerC1();

        static TypeId
        GetTypeId();

        // virtual void
        //         StartApplication(); // Called at time specified by Start

        void SendPacket();

        virtual void
            OnData(shared_ptr<const Data> contentObject);

        void ServiceDiscovery();

        void ScheduleNextPacket();


        private:
            Name producer_name;
            bool producer_outdated = false;
    };
}
}