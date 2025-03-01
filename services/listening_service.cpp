#include <filesystem>

#include "../constants/network.h"
#include "../constants/hardware.h"
#include "ir_command.pb.h"
#include "listening_service.h"
#include "status_led_mgr.h"

using Clock     = std::chrono::high_resolution_clock;
using TimePoint = std::chrono::time_point<Clock>;

namespace bai  = boost::asio::ip;

void ListeningService::run(const std::string& name) {
    ListeningService listening_service(name,
                                       bai::make_address(CMD_RECEIVE_ADDR),
                                       CMD_RECEIVE_PORT);

    while ( true )
        if ( listening_service.receiveNetworkCommand() )
            listening_service.issueIrCommand();
}

ListeningService::ListeningService(const std::string&  name,
                                   const bai::address& mc_addr,
                                   uint16_t            mc_port):
    name(name),
    io_ctx(),
    mcast_ep(mc_addr, mc_port),
    socket(io_ctx, mcast_ep)
{
    socket.set_option(bai::multicast::join_group(mcast_ep.address()));
}

bool ListeningService::receiveNetworkCommand() {
    std::array<char, 1024> buffer;

    size_t rcvd_bytes = socket.receive(boost::asio::buffer(buffer));
    if ( rcvd_bytes ) {
        IrRelay::IrCommand command_msg;
        command_msg.ParseFromString( std::string(buffer.data(), rcvd_bytes) );

        int interval_count = command_msg.interval_size();
        if ( command_msg.name() == name &&
             interval_count > 1 && interval_count % 2 != 0 ) {
            cmd_deltas.clear();
            for ( int i=0; i < interval_count; i++ ) {
                cmd_deltas.push_back( command_msg.interval(i) );
            }

            return true;
        }
    }

    return false;
}

void ListeningService::busyWaitUntil(const TimePoint& go_time) {
    while( Clock::now() < go_time );
}

gpiod::line::value operator!(gpiod::line::value val) {
    if ( val == gpiod::line::value::INACTIVE )
        return gpiod::line::value::ACTIVE;
    return gpiod::line::value::INACTIVE;
}

void ListeningService::issueIrCommand() {
    auto settings  = gpiod::line_settings()
                        .set_direction(gpiod::line::direction::OUTPUT);
    auto ir_offset = gpiod::line::offset(IR_OUTPUT);
    auto ir_snd_lr = gpiod::chip(std::filesystem::path(GPIO_CHIP_PATH))
                        .prepare_request()
                        .set_consumer("ir_output")
                        .add_line_settings(ir_offset, settings)
                        .do_request();

    ir_snd_lr.set_value(ir_offset, gpiod::line::value::INACTIVE);

    // Build the list of times at which we will toggle the IR LED.
    //  To allow computation time, anticipate starting 10ms in the future;
    std::vector<TimePoint> toggle_times;
    toggle_times.push_back( Clock::now() + std::chrono::milliseconds(10) );
    for ( auto delay: cmd_deltas )
        toggle_times.push_back( toggle_times.back() +
                                std::chrono::duration<uint64_t, std::nano>(delay) );

    TimePoint          tp;
    gpiod::line::value clv;

    busyWaitUntil( toggle_times.front() );
    for ( int i=0; i < (toggle_times.size()-1); i++ ) {
        if ( i % 2 == 0 ) { // HIGH
            tp  = toggle_times[i];
            clv = gpiod::line::value::ACTIVE;
            ir_snd_lr.set_value(ir_offset, clv);
            while ( tp < toggle_times[i+1] ) {
                // Wait half of the 26.316 us period (38kHz)
                busyWaitUntil( tp + std::chrono::nanoseconds(13158) );
                tp  = Clock::now();
                clv = !clv;
                ir_snd_lr.set_value( ir_offset, clv );
            }
        } else { // LOW
            ir_snd_lr.set_value(ir_offset, gpiod::line::value::INACTIVE);
            busyWaitUntil(toggle_times[i+1]);
        }
    }
    ir_snd_lr.set_value(ir_offset, gpiod::line::value::INACTIVE);

    StatusLedMgr::addToGreen(1);
}

