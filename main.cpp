#include <boost/program_options.hpp>
#include <google/protobuf/stubs/common.h>
#include <thread>

#include "constants/network.h"
#include "services/presence_notifier.h"
#include "services/learning_service.h"
#include "services/listening_service.h"
#include "services/status_led_mgr.h"

namespace bpo = boost::program_options;

void runDaemon(const std::string& name, const std::string& description) {
    GOOGLE_PROTOBUF_VERIFY_VERSION;

    boost::asio::io_context ioc;
    StatusLedMgr::initialize(ioc);
    PresenceNotifier::initialize(ioc, name, description);

    std::thread learning_thread(LearningService::run, name);
    std::thread listening_thread(ListeningService::run, name);

    // Start Indicator; 3 x (GREEN + BLUE + OFF)
    StatusLedMgr::addToGreen(3);
    StatusLedMgr::addToBlue(3);

    ioc.run();

    google::protobuf::ShutdownProtobufLibrary();
}

int main(int argc, char** argv) {
    std::string name;
    std::string description;

    bpo::options_description opt_desc("Options");
    opt_desc.add_options()
        ("help,h",
            "This help text")
        ("name,n",
            bpo::value<std::string>(&name), "This relay's name")
        ("description,d",
            bpo::value<std::string>(&description), "This relay's description")
    ;

    bpo::variables_map var_map;
    bpo::store( bpo::parse_command_line(argc, argv, opt_desc), var_map );
    bpo::notify( var_map );

    if ( var_map.count("help") ) {
        std::cout << opt_desc << std::endl;
        return 0;
    }

    if ( !var_map.count("name") || !var_map.count("description") ) {
        std::cerr << "Failed to start relay: 'name' AND 'description' "
                  << "must be specified!\n";
        return 1;
    }

    runDaemon(name, description);

    return 0;
}
