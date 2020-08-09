/*
 * MIT License
 * 
 * Copyright (c) 2020 Guanyu Zhou
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "gzat_parser.hpp"
#include <gtest/gtest.h>

using namespace gzat;

TEST(GZAT_Parse, Construct_No_MS)
{
    AtCommand atcmd("ATZ");
    EXPECT_EQ(atcmd.ms, 0);
    EXPECT_EQ(atcmd.me, 0);
    EXPECT_STREQ(atcmd.cmd_id.c_str(), "Z");
    EXPECT_TRUE(atcmd.cmd_payload.empty());
    EXPECT_STREQ(atcmd.GetRawCommand().c_str(), "ATZ");
}

TEST(GZAT_Parse, Construct_MS)
{
    AtCommand atcmd("AT+Z");
    EXPECT_EQ(atcmd.ms, 1);
    EXPECT_EQ(atcmd.me, 0);
    EXPECT_STREQ(atcmd.cmd_id.c_str(), "+Z");
    EXPECT_TRUE(atcmd.cmd_payload.empty());
    EXPECT_STREQ(atcmd.GetRawCommand().c_str(), "AT+Z");
}

TEST(GZAT_Parse, Construct_MS_Test)
{
    AtCommand atcmd("AT#Z=?");
    EXPECT_EQ(atcmd.ms, 2);
    EXPECT_EQ(atcmd.me, 1);
    EXPECT_STREQ(atcmd.cmd_id.c_str(), "#Z");
    EXPECT_TRUE(atcmd.cmd_payload.empty());
    EXPECT_STREQ(atcmd.GetRawCommand().c_str(), "AT#Z=?");
}

TEST(GZAT_Parse, Construct_MS_Get)
{
    AtCommand atcmd("AT+CSQ?");
    EXPECT_EQ(atcmd.ms, 1);
    EXPECT_EQ(atcmd.me, 2);
    EXPECT_STREQ(atcmd.cmd_id.c_str(), "+CSQ");
    EXPECT_TRUE(atcmd.cmd_payload.empty());
    EXPECT_STREQ(atcmd.GetRawCommand().c_str(), "AT+CSQ?");
}

TEST(GZAT_Parse, Construct_MS_Set)
{
    AtCommand atcmd("AT+ABC=1,\"abc\"");
    EXPECT_EQ(atcmd.ms, 1);
    EXPECT_EQ(atcmd.me, 3);
    EXPECT_STREQ(atcmd.cmd_id.c_str(), "+ABC");
    EXPECT_STREQ(atcmd.cmd_payload.c_str(), "1,\"abc\"");
    EXPECT_STREQ(atcmd.GetRawCommand().c_str(), "AT+ABC=1,\"abc\"");
}

TEST(GZAT_Parse, Parse_MS_Get_Int)
{
    AtCommand atcmd("AT+CSQ?");
    CommandParser p(atcmd);
    std::shared_ptr<CommaSplitParser> p1 = std::make_shared<CommaSplitParser>(0);
    int64_t p1_int = 0;
    p1->AddIntegerOutput(&p1_int);
    std::shared_ptr<CommaSplitParser> p2 = std::make_shared<CommaSplitParser>(1);
    int64_t p2_int = 0;
    p2->AddIntegerOutput(&p2_int);
    p.AddChildParser(p1).AddChildParser(p2);
    p.Parse("+CSQ: 10,100");
    EXPECT_EQ(p1_int, 10);
    EXPECT_EQ(p2_int, 100);
}

TEST(GZAT_Parse, Parse_MS_Get_String)
{
    AtCommand atcmd("AT+PDP?");
    CommandParser p(atcmd);
    std::shared_ptr<CommaSplitParser> p1 = std::make_shared<CommaSplitParser>(0);
    int64_t p1_int = 0;
    p1->AddIntegerOutput(&p1_int);
    std::shared_ptr<CommaSplitParser> p2 = std::make_shared<CommaSplitParser>(1);
    std::string p2_string;
    p2->AddStringOutput(&p2_string);
    std::shared_ptr<CommaSplitParser> p3 = std::make_shared<CommaSplitParser>(2);
    std::string p3_string;
    p3->AddStringOutput(&p3_string);
    p.AddChildParser(p1).AddChildParser(p2).AddChildParser(p3);
    p.Parse("+PDP: 10,\"1.2.3.4\",abc\r\rOK");
    EXPECT_EQ(p1_int, 10);
    EXPECT_STREQ(p2_string.c_str(), "1.2.3.4");
    EXPECT_STREQ(p3_string.c_str(), "abc");
}