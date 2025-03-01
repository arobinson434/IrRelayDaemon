#pragma once

#include <boost/asio.hpp>
#include <gpiod.hpp>

class ListeningService {
    public:
        /**
         * Create and start a ListeningService object on the current thread,
         *
         * This function NEVER returns. If you need to do work after invoking
         *  this function, you should run it in it's own thread.
         */
        static void run(const std::string& name);

        bool receiveNetworkCommand();
        void issueIrCommand();

        ListeningService(const ListeningService&)            = delete;
        ListeningService(ListeningService&&)                 = delete;
        ListeningService& operator=(const ListeningService&) = delete;
        ListeningService& operator=(ListeningService&&)      = delete;

    private:
        ListeningService(const std::string&              name,
                         const boost::asio::ip::address& mc_ep,
                         uint16_t                        mc_port);

        void busyWaitUntil(const std::chrono::time_point<std::chrono::high_resolution_clock>& go_time);

        std::string                    name;
        boost::asio::io_context        io_ctx; // TODO: remove this
        boost::asio::ip::udp::endpoint mcast_ep;
        boost::asio::ip::udp::socket   socket;

        std::vector<uint64_t>          cmd_deltas;
};

