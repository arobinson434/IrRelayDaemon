#include <iostream>

#include "../constants/network.h"
#include "presence_notifier.h"
#include "presence_notification.pb.h"

PresenceNotifier* PresenceNotifier::instance_ptr = nullptr;

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

PresenceNotifier::~PresenceNotifier() {
    if ( instance_ptr )
        delete instance_ptr;
}

void PresenceNotifier::initialize(boost::asio::io_context& ioc,
                                  const std::string&       name,
                                  const std::string&       description) {
    if ( !instance_ptr )
        instance_ptr = new PresenceNotifier(
            ioc, boost::asio::ip::make_address(PRESENCE_NOTIFIER_ADDR),
            PRESENCE_NOTIFIER_PORT, name, description);
    else
        throw std::runtime_error(
            "Attempted to double initialize PresenceNotifier!!!");
}

void PresenceNotifier::publishNotification() {
    socket.async_send_to(
        boost::asio::buffer(msg_buffer.data(), msg_buffer.size()),
        mcast_ep,
        std::bind(&PresenceNotifier::waitToPublish, this)
    );
}

void PresenceNotifier::waitToPublish() {
    timer.expires_after(std::chrono::seconds(5));
    timer.async_wait( std::bind(&PresenceNotifier::publishNotification, this) );
}

