#include "status_led_mgr.h"
#include "../constants/hardware.h"

const std::filesystem::path gpio_path(GPIO_CHIP_PATH);
const gpiod::line::offsets  led_pins{ RED_LED, GREEN_LED, BLUE_LED };

StatusLedMgr* StatusLedMgr::instance_ptr = nullptr;

void StatusLedMgr::initialize(boost::asio::io_context& ioc) {
    if ( !instance_ptr )
        instance_ptr = new StatusLedMgr(ioc);
    else
        throw std::runtime_error(
            "Attempted to double initialize StatusLedMgr!!!");
}

void StatusLedMgr::addToRed(uint8_t count) {
    if ( !instance_ptr )
        throw std::runtime_error(
            "StatusLedMgr: 'addToRed' called on uninitialized mgr!");

    std::lock_guard<std::mutex> g(instance_ptr->state_lock);
    instance_ptr->red_count += count;
}

void StatusLedMgr::addToGreen(uint8_t count) {
    if ( !instance_ptr )
        throw std::runtime_error(
            "StatusLedMgr: 'addToGreen' called on uninitialized mgr!");

    std::lock_guard<std::mutex> g(instance_ptr->state_lock);
    instance_ptr->green_count += count;
}

void StatusLedMgr::addToBlue(uint8_t count) {
    if ( !instance_ptr )
        throw std::runtime_error(
            "StatusLedMgr: 'addToBlue' called on uninitialized mgr!");

    std::lock_guard<std::mutex> g(instance_ptr->state_lock);
    instance_ptr->blue_count += count;
}

void StatusLedMgr::setBlueOn(bool on) {
    if ( !instance_ptr )
        throw std::runtime_error(
            "StatusLedMgr: 'setBlueOn' called on uninitialized mgr!");

    std::lock_guard<std::mutex> g(instance_ptr->state_lock);
    instance_ptr->blue_on = on;
}

void StatusLedMgr::allOff() {
    led_lr.set_values( led_pins, {
        gpiod::line::value::INACTIVE,
        gpiod::line::value::INACTIVE,
        gpiod::line::value::INACTIVE
    });
}

StatusLedMgr::StatusLedMgr(boost::asio::io_context& ioc):
    ioc(ioc), timer(ioc), count(0),
    red_count(0), green_count(0), blue_count(0), blue_on(false),
    led_lr(std::move( gpiod::chip(gpio_path)
                        .prepare_request()
                        .set_consumer("status_led")
                        .add_line_settings(
                            led_pins,
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
    allOff();

    {
        std::lock_guard<std::mutex> g(state_lock);

        if ( count % 3 == 0 && red_count > 0 ) { // RED
            red_count--;
            led_lr.set_value(gpiod::line::offset(RED_LED),
                gpiod::line::value::ACTIVE);
        } else if ( count % 3 == 1 && green_count > 0 ) { // GREEN
            green_count--;
            led_lr.set_value(gpiod::line::offset(GREEN_LED),
                gpiod::line::value::ACTIVE);
        } else if ( count % 3 == 2 && (blue_count > 0 || blue_on) ) { // BLUE
            if ( blue_count > 0 )
                blue_count--;

            led_lr.set_value(gpiod::line::offset(BLUE_LED),
                gpiod::line::value::ACTIVE);
        }
    }

    count++;

    timer.expires_after(std::chrono::milliseconds(200));
    timer.async_wait( std::bind(&StatusLedMgr::tickHandler, this) );
}

