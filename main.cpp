#include <google/protobuf/stubs/common.h>
#include <thread>

#include "services/presence_notifier.h"
#include "services/learning_service.h"

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

    std::thread learning_thread(LearningService::run, "Living Room");

    PresenceNotifier::run("Living Room", "IR Relay in the Living Room");

    google::protobuf::ShutdownProtobufLibrary();

    return 0;
}
