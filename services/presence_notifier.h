#pragma once

#include <boost/asio.hpp>

class PresenceNotifier {
    public:
        /**
         * Create and start a PresenceNotifier object on the current thread,
         *  eliding the need to provide a boost IO context.
         *
         * As the context is created and run locally, this function NEVER
         *  returns. If you need to do work after invoking this function, you
         *  should run it in it's own thread.
         */
        static void run(const std::string& name,
                        const std::string& description);

        PresenceNotifier(boost::asio::io_context&        ioc,
                         const boost::asio::ip::address& mc_addr,
                         uint16_t                        mc_port,
                         const std::string&              name,
                         const std::string&              description);

    private:
        void publishNotification();
        void waitToPublish();

        boost::asio::ip::udp::endpoint mcast_ep;
        boost::asio::ip::udp::socket   socket;
        boost::asio::steady_timer      timer;
        std::vector<char>              msg_buffer;
};

