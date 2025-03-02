#include <google/protobuf/stubs/common.h>
#include <thread>

#include "constants/network.h"
#include "services/presence_notifier.h"
#include "services/learning_service.h"
#include "services/listening_service.h"
#include "services/status_led_mgr.h"

int main() {
    GOOGLE_PROTOBUF_VERIFY_VERSION;

    // TODO: read input .ini file for name and description

    boost::asio::io_context ioc;

    StatusLedMgr::initialize(ioc);
    PresenceNotifier::initialize(ioc, "Living Room", 
        "IR Relay in the Living Room");

    std::thread learning_thread(LearningService::run, "Living Room");
    std::thread listening_thread(ListeningService::run, "Living Room");

    // Start Indicator; 2 x (GREEN + BLUE + OFF)
    StatusLedMgr::addToGreen(2);
    StatusLedMgr::addToBlue(2);

    ioc.run();

    google::protobuf::ShutdownProtobufLibrary();

    return 0;
}
