#include "ws_parser_test.h"
#include "http_ws.h"
#include "websocket_parser.h"

namespace tcp
{
namespace http_ws
{
WSParserTest::WSParserTest() : ws_parser_()
{

}

WSParserTest::~WSParserTest()
{

}

void WSParserTest::SetUp()
{

}

void WSParserTest::TearDown()
{

}

void WSParserTest::Test001()
{
    const int frame_type = ws::TEXT_FRAME;
    const char data[] = "hello, world";
    const size_t len = strlen(data);

    const int flags = (ws::TEXT_FRAME == frame_type ? WS_OP_TEXT : WS_OP_BINARY) | WS_FINAL_FRAME;
    const size_t frame_len = websocket_calc_frame_size((websocket_flags) flags, len);

    char* frame = (char*) malloc(frame_len);
    if (nullptr == frame)
    {
        FAIL() << "failed to alloc memory";
    }

    websocket_build_frame(frame, (websocket_flags) flags, nullptr, (const char*) data, len);

    ws_parser_.Execute(frame, frame_len);
    ws_parser_.Execute(frame, frame_len);

    free(frame);
}

void WSParserTest::Test002()
{
    const int frame_type = ws::BINARY_FRAME;
    const char data[] = "hello, world";
    const size_t len = strlen(data);

    const int flags = (ws::TEXT_FRAME == frame_type ? WS_OP_TEXT : WS_OP_BINARY) | WS_FINAL_FRAME;
    const size_t frame_len = websocket_calc_frame_size((websocket_flags) flags, len);

    char* frame = (char*) malloc(frame_len);
    if (nullptr == frame)
    {
        FAIL() << "failed to alloc memory";
    }

    websocket_build_frame(frame, (websocket_flags) flags, nullptr, (const char*) data, len);

    ws_parser_.Execute(frame, frame_len);
    ws_parser_.Execute(frame, frame_len);

    free(frame);
}

void WSParserTest::Test003()
{
    const int frame_type = ws::TEXT_FRAME;
    const char data[] = "hello, world";
    const size_t len = strlen(data);

    std::string total;

    // frame 1
    int flags = (ws::TEXT_FRAME == frame_type ? WS_OP_TEXT : WS_OP_BINARY);
    size_t frame_len = websocket_calc_frame_size((websocket_flags) flags, len);

    char* frame = (char*) malloc(frame_len);
    if (nullptr == frame)
    {
        FAIL() << "failed to alloc memory";
    }

    websocket_build_frame(frame, (websocket_flags) flags, nullptr, (const char*) data, len);
    total.append(frame, frame_len);
    free(frame);

    // frame 2
    flags = (ws::TEXT_FRAME == frame_type ? WS_OP_TEXT : WS_OP_BINARY) | WS_FINAL_FRAME;
    frame_len = websocket_calc_frame_size((websocket_flags) flags, len);

    frame = (char*) malloc(frame_len);
    if (nullptr == frame)
    {
        FAIL() << "failed to alloc memory";
    }

    websocket_build_frame(frame, (websocket_flags) flags, nullptr, (const char*) data, len);
    total.append(frame, frame_len);
    free(frame);

    ws_parser_.Execute(total.data(), total.size());
}

void WSParserTest::Test004()
{

}

ADD_TEST_F(WSParserTest, Test001);
ADD_TEST_F(WSParserTest, Test002);
ADD_TEST_F(WSParserTest, Test003);
ADD_TEST_F(WSParserTest, Test004);
}
}
