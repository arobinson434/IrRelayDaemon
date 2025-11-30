#pragma once

#include <boost/asio.hpp>
#include <gpiod.hpp>

class StatusLedMgr {
    public:
        static void initialize(boost::asio::io_context& ioc);

        static void add(uint8_t count);

        StatusLedMgr(const StatusLedMgr&)            = delete;
        StatusLedMgr(StatusLedMgr&&)                 = delete;
        StatusLedMgr& operator=(const StatusLedMgr&) = delete;
        StatusLedMgr& operator=(StatusLedMgr&&)      = delete;

        ~StatusLedMgr();

    private:
        StatusLedMgr(boost::asio::io_context& ioc);

        void tickHandler();

        static StatusLedMgr*      instance_ptr;
        boost::asio::io_context&  ioc;
        boost::asio::steady_timer timer;

        uint32_t count;
        uint32_t blink_count;

        gpiod::line_request led_lr;

        // Lock for the publicly modifiable state;
        std::mutex state_lock;
};

