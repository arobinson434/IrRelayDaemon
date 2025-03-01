#include <google/protobuf/stubs/common.h>
#include <thread>

#include "constants/network.h"
#include "services/presence_notifier.h"
#include "services/learning_service.h"
#include "services/listening_service.h"
#include "services/status_led_mgr.h"

// This application runs across three threads (including this main thread).
//  Thread #1
//      This main thread, in addition to spawning the other two threads,
//      primarily runs the notifier service, which multicasts this relay's
//      name and description.
//  Thread #2
//      This thread runs the learning service. It waits for a button press,
//      listends for an IR signal, and then multicasts that new signal onto the
//      connected network.
//  Thread #3
//      This thread runs the command service. It listens to the network for
//      incloming requests (commands). When a command is received, the IR signal
//      described in the command message will be issued via the IR transmitter.

int main() {
    GOOGLE_PROTOBUF_VERIFY_VERSION;

    // TODO: read input .ini file for name and description

    boost::asio::io_context ioc;
    PresenceNotifier notifier(
        ioc, boost::asio::ip::make_address(PRESENCE_NOTIFIER_ADDR),
        PRESENCE_NOTIFIER_PORT,
        "Living Room",
        "IR Relay in the Living Room"
    );
    StatusLedMgr::initialize(ioc);

    std::thread learning_thread(LearningService::run, "Living Room");
    std::thread listening_thread(ListeningService::run, "Living Room");

    // Start Indicator; 2 x (GREEN + BLUE + OFF)
    StatusLedMgr::addToGreen(2);
    StatusLedMgr::addToBlue(2);

    ioc.run();

    google::protobuf::ShutdownProtobufLibrary();

    return 0;
}
