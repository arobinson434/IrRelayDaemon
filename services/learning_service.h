#pragma once

#include <boost/asio.hpp>
#include <gpiod.hpp>
#include <mutex>

class LearningService {
    public:
        /**
         * Create and start a LearningService object on the current thread,
         *
         * This function NEVER returns. If you need to do work after invoking
         *  this function, you should run it in it's own thread.
         */
        static void run(const std::string& name, std::mutex& ir_op);

        LearningService(const LearningService&)            = delete;
        LearningService(LearningService&&)                 = delete;
        LearningService& operator=(const LearningService&) = delete;
        LearningService& operator=(LearningService&&)      = delete;

    private:
        LearningService(const std::string&              name,
                        const boost::asio::ip::address& mc_ep,
                        uint16_t                        mc_port,
                        std::mutex&                     ir_op );

        void receiveIrCommand();
        void publishIrCommand();

        std::string                    name;
        boost::asio::io_context        io_ctx;
        boost::asio::ip::udp::endpoint mcast_ep;
        boost::asio::ip::udp::socket   socket;
        std::mutex&                    ir_operation;

        std::vector<uint64_t>          cmd_deltas;
};

