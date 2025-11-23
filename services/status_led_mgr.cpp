#include "status_led_mgr.h"
#include "../constants/hardware.h"

const std::filesystem::path gpio_path(GPIO_CHIP_PATH);

StatusLedMgr* StatusLedMgr::instance_ptr = nullptr;

void StatusLedMgr::initialize(boost::asio::io_context& ioc) {
    if ( !instance_ptr )
        instance_ptr = new StatusLedMgr(ioc);
    else
        throw std::runtime_error(
            "Attempted to double initialize StatusLedMgr!!!");
}

void StatusLedMgr::add(uint8_t count) {
    if ( !instance_ptr )
        throw std::runtime_error(
            "StatusLedMgr: 'add' called on uninitialized mgr!");

    std::lock_guard<std::mutex> g(instance_ptr->state_lock);
    instance_ptr->blink_count += count;
}

StatusLedMgr::StatusLedMgr(boost::asio::io_context& ioc):
    ioc(ioc), timer(ioc), count(0), blink_count(0),
    led_lr(std::move( gpiod::chip(gpio_path)
                        .prepare_request()
                        .set_consumer("status_led")
                        .add_line_settings(
                            gpiod::line::offset(STATUS_LED),
                            gpiod::line_settings()
                                .set_direction(gpiod::line::direction::OUTPUT)
                        )
                        .do_request() ))
{ 
    tickHandler();
}

StatusLedMgr::~StatusLedMgr() {
    if ( instance_ptr )
        delete instance_ptr;
}

void StatusLedMgr::tickHandler() {
    led_lr.set_value(gpiod::line::offset(STATUS_LED),
        gpiod::line::value::INACTIVE);

    {
        std::lock_guard<std::mutex> g(state_lock);

        if ( count % 2 == 0 && blink_count > 0 ) {
            blink_count--;
            led_lr.set_value(gpiod::line::offset(STATUS_LED),
                gpiod::line::value::ACTIVE);
        }
    }

    count++;

    timer.expires_after(std::chrono::milliseconds(200));
    timer.async_wait( std::bind(&StatusLedMgr::tickHandler, this) );
}

