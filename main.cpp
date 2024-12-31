#include <boost/asio.hpp>
#include <google/protobuf/stubs/common.h>

#include "network/presence_notifier.h"

#define MCAST_ADDR "225.1.2.3"
#define MCAST_PORT 5000
#define MAX_BUFF   1024

int main() {
    GOOGLE_PROTOBUF_VERIFY_VERSION;

    boost::asio::io_context ioc;

    PresenceNotifier notifier(
        ioc, boost::asio::ip::make_address(MCAST_ADDR), MCAST_PORT,
        "Living Room", "IR Relay in the Living Room"
    );

    ioc.run();

    google::protobuf::ShutdownProtobufLibrary();

    return 0;
}
