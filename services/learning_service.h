#pragma once

#include <boost/asio.hpp>
#include <gpiod.hpp>

class LearningService {
    public:
        /**
         * Create and start a LearningService object on the current thread,
         *
         * This function NEVER returns. If you need to do work after invoking
         *  this function, you should run it in it's own thread.
         */
        static void run(const std::string& name);

        void waitOnButton();
        bool receiveIrCommand();
        void publishIrCommand();

        LearningService(const LearningService&)            = delete;
        LearningService(LearningService&&)                 = delete;
        LearningService& operator=(const LearningService&) = delete;
        LearningService& operator=(LearningService&&)      = delete;

    private:
        LearningService(const std::string&              name,
                        const boost::asio::ip::address& mc_ep,
                        uint16_t                        mc_port);

        std::string                    name;
        boost::asio::io_context        io_ctx;
        boost::asio::ip::udp::endpoint mcast_ep;
        boost::asio::ip::udp::socket   socket;

        std::vector<uint64_t>          cmd_deltas;
};

