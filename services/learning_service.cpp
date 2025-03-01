#include <filesystem>

#include "../constants/network.h"
#include "../constants/hardware.h"
#include "ir_command.pb.h"
#include "learning_service.h"
#include "status_led_mgr.h"

namespace bai  = boost::asio::ip;

void LearningService::run(const std::string& name) {
    LearningService learning_service(name,
                                     bai::make_address(CMD_ADVERTISE_ADDR),
                                     CMD_ADVERTISE_PORT);

    while ( true ) {
        learning_service.waitOnButton();
        if ( learning_service.receiveIrCommand() )
            learning_service.publishIrCommand();
    }
}

LearningService::LearningService(const std::string&  name,
                                 const bai::address& mc_addr,
                                 uint16_t            mc_port):
    name(name),
    io_ctx(),
    mcast_ep(mc_addr, mc_port),
    socket(io_ctx, mcast_ep.protocol())
{ }

void LearningService::waitOnButton() {
    auto settings   = gpiod::line_settings()
                        .set_direction(gpiod::line::direction::INPUT)
                        .set_edge_detection(gpiod::line::edge::FALLING)
                        .set_bias(gpiod::line::bias::PULL_UP);
    auto btn_offset = gpiod::line::offset(INPUT_BUTTON);
    auto button_lr  = gpiod::chip(std::filesystem::path(GPIO_CHIP_PATH))
                        .prepare_request()
                        .set_consumer("button_input")
                        .add_line_settings(btn_offset, settings)
                        .do_request();

    button_lr.wait_edge_events(std::chrono::seconds(-1));

    StatusLedMgr::setBlueOn(true);
}

bool LearningService::receiveIrCommand() {
    bool                  success = false;
    std::vector<uint64_t> timestamps;

    auto settings   = gpiod::line_settings()
                        .set_direction(gpiod::line::direction::INPUT)
                        .set_edge_detection(gpiod::line::edge::BOTH)
                        .set_bias(gpiod::line::bias::PULL_UP);
    auto ir_offset  = gpiod::line::offset(IR_INPUT);
    auto ir_rcv_lr  = gpiod::chip(std::filesystem::path(GPIO_CHIP_PATH))
                        .prepare_request()
                        .set_consumer("ir_input")
                        .add_line_settings(ir_offset, settings)
                        .do_request();

    if ( ir_rcv_lr.wait_edge_events(std::chrono::seconds(30)) ) {
        gpiod::edge_event_buffer buffer(100);

        while ( ir_rcv_lr.wait_edge_events(std::chrono::milliseconds(65)) ) {
            ir_rcv_lr.read_edge_events(buffer);

            for( auto event: buffer )
                timestamps.push_back(event.timestamp_ns().ns());
        }

        if ( timestamps.size() > 1 && timestamps.size() % 2 == 0 ) {
            cmd_deltas.clear();
            for (int index=1; index < timestamps.size(); index++)
                cmd_deltas.push_back(timestamps[index] - timestamps[index-1]);

            success = true;
        } else {
            std::cerr << "Learning Service: Bad Signal Received!";

            StatusLedMgr::setBlueOn(false);
            StatusLedMgr::addToRed(2);
        }
    } else {
        std::cerr << "Learning Service: No Signal Received!";

        StatusLedMgr::setBlueOn(false);
        StatusLedMgr::addToRed(1);
    }

    return success;
}

void LearningService::publishIrCommand() {
    if ( cmd_deltas.empty() )
        return; // This shouldn't happen;

    IrRelay::IrCommand command_msg;
    std::vector<char>  msg_buffer;

    command_msg.set_name(name);
    for( auto delta: cmd_deltas )
        command_msg.add_interval(delta);

    msg_buffer.resize(command_msg.ByteSizeLong());
    command_msg.SerializeToArray(msg_buffer.data(), msg_buffer.size());

    auto sent_bytes = socket.send_to(
        boost::asio::buffer(msg_buffer.data(), msg_buffer.size()),
        mcast_ep
    );

    StatusLedMgr::setBlueOn(false);

    if ( sent_bytes != msg_buffer.size() ) {
        std::cerr << "Learning Service: Failed to publish command!";
        StatusLedMgr::addToRed(3);
    } else {
        StatusLedMgr::addToGreen(2);
    }
}
