#include <iostream>

#include "../constants/network.h"
#include "presence_notifier.h"
#include "presence_notification.pb.h"

using boost::asio::ip::udp;

PresenceNotifier::PresenceNotifier(boost::asio::io_context&        ioc,
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

void PresenceNotifier::run(const std::string& name,
                           const std::string& description) {
    boost::asio::io_context ioc;

    PresenceNotifier notifier(
        ioc, boost::asio::ip::make_address(PRESENCE_NOTIFIER_ADDR),
        PRESENCE_NOTIFIER_PORT, name, description
    );

    ioc.run();
}

void PresenceNotifier::publishNotification() {
    socket.async_send_to(
        boost::asio::buffer(msg_buffer.data(), msg_buffer.size()),
        mcast_ep,
        [this](boost::system::error_code ec, std::size_t) {
            if ( ec )
                std::cerr << "Presence Notifier: "
                          << "Failed to send presence notification!"
                          << "\n\tError Code: " << ec << std::endl;

            waitToPublish();
        }
    );
}

void PresenceNotifier::waitToPublish() {
    timer.expires_after(std::chrono::seconds(5));
    timer.async_wait(
        [this](boost::system::error_code ec) {
            publishNotification();
        }
    );
}

