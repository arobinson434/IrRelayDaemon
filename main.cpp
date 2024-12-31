#include <iostream>
#include <boost/asio.hpp>

#include "gen/presence_notification.pb.h"

#define MCAST_ADDR "225.1.2.3"
#define MCAST_PORT 5000
#define MAX_BUFF   1024

using boost::asio::ip::udp;

class PresenceNotifier {
    public:
        PresenceNotifier(boost::asio::io_context&        ioc,
                         const boost::asio::ip::address& mc_addr,
                         uint16_t                        mc_port,
                         const std::string&              name,
                         const std::string&              description):
            mcast_ep(mc_addr, mc_port),
            socket(ioc, mcast_ep.protocol()),
            timer(ioc)
        {
            IrRelay::PresenceNotification notification;
            notification.set_name(name);   
            notification.set_description(description);

            msg_buffer.resize(notification.ByteSizeLong());
            notification.SerializeToArray(msg_buffer.data(), msg_buffer.size());

            publishNotification();
        }

    private:
        void publishNotification() {
            socket.async_send_to(
                boost::asio::buffer(msg_buffer.data(), msg_buffer.size()),
                mcast_ep,
                [this](boost::system::error_code ec, std::size_t) {
                    if ( ec )
                        std::cerr << "Failed to send presence notification!"
                                  << " Error Code: " << ec << std::endl;

                    waitToPublish();
                }
            );
        }

        void waitToPublish() {
            timer.expires_after(std::chrono::seconds(1));
            timer.async_wait(
                [this](boost::system::error_code ec) {
                    publishNotification();
                }
            );
        }

        udp::endpoint             mcast_ep;
        udp::socket               socket;
        boost::asio::steady_timer timer;
        std::vector<char>         msg_buffer;
};

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
