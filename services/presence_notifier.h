#pragma once

#include <boost/asio.hpp>

class PresenceNotifier {
    public:
        static void initialize(boost::asio::io_context& ioc,
                               const std::string&       name,
                               const std::string&       description);

        PresenceNotifier(const PresenceNotifier&)            = delete;
        PresenceNotifier(PresenceNotifier&&)                 = delete;
        PresenceNotifier& operator=(const PresenceNotifier&) = delete;
        PresenceNotifier& operator=(PresenceNotifier&&)      = delete;

        ~PresenceNotifier();

    private:
        PresenceNotifier(boost::asio::io_context&        ioc,
                         const boost::asio::ip::address& mc_addr,
                         uint16_t                        mc_port,
                         const std::string&              name,
                         const std::string&              description);

        static PresenceNotifier* instance_ptr;

        void publishNotification();
        void waitToPublish();

        boost::asio::ip::udp::endpoint mcast_ep;
        boost::asio::ip::udp::socket   socket;
        boost::asio::steady_timer      timer;
        std::vector<char>              msg_buffer;
};

