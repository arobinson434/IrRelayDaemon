#pragma once

#include <boost/asio.hpp>
#include <gpiod.hpp>

class StatusLedMgr {
    public:
        static void initialize(boost::asio::io_context& ioc);

        static void addToRed(uint8_t count);
        static void addToGreen(uint8_t count);
        static void addToBlue(uint8_t count);
        static void setBlueOn(bool on);

        StatusLedMgr(const StatusLedMgr&)            = delete;
        StatusLedMgr(StatusLedMgr&&)                 = delete;
        StatusLedMgr& operator=(const StatusLedMgr&) = delete;
        StatusLedMgr& operator=(StatusLedMgr&&)      = delete;

        ~StatusLedMgr();

    private:
        StatusLedMgr(boost::asio::io_context& ioc);

        void tickHandler();
        void allOff();

        static StatusLedMgr*      instance_ptr;
        boost::asio::io_context&  ioc;
        boost::asio::steady_timer timer;

        uint32_t count;
        uint8_t  red_count;
        uint8_t  green_count;
        uint8_t  blue_count;
        bool     blue_on;

        gpiod::line_request led_lr;

        // Lock for the publicly modifiable state;
        //  Ie: red_count, green_count, blue_count & blue_on
        std::mutex state_lock;
};

