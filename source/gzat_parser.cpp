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
#include <iostream>

namespace gzat {

    AtCommand::AtCommand() {

    }

    AtCommand::AtCommand(const std::string & raw_cmd) {
        ms = 0U;
        me = 0U;
        // Check start phase
        if(raw_cmd.substr(0, 2) == "AT") {
            // Check start marker
            size_t byte = 2U;
            if(raw_cmd.size() > 2) {
                for(uint8_t id = 1U; id < 7; id++) {
                    if(raw_cmd.substr(2,1) == MS_LUT[id]) {
                        ms = id;
                        byte++;
                        break;
                    }
                }

                for(uint8_t id = 1U; id < 6; id++) {
                    const size_t me_pos = raw_cmd.substr(byte).find(ME_LUT[id]);
                    if(me_pos != std::string::npos) {
                        me = id;
                        cmd_id = MS_LUT[ms]+raw_cmd.substr(byte, me_pos);
                        byte = byte + me_pos + ME_LUT[id].size();
                        break;
                    }
                }

                if(me == 0U) {
                    cmd_id = MS_LUT[ms]+raw_cmd.substr(byte);
                }
                else {
                    cmd_payload = raw_cmd.substr(byte);
                }
            }
        }
    }

    std::string AtCommand::GetRawCommand() const {
        return "AT"+cmd_id+ME_LUT[me]+cmd_payload;
    }

    Parser::Parser() {
        int_out = nullptr;
        double_out = nullptr;
        string_out = nullptr;
    }

    Parser& Parser::AddChildParser(const std::shared_ptr<Parser> parser) {
        child_parsers.push_back(parser);
        return *this;
    }

    Parser& Parser::AddIntegerOutput(int64_t * const out) {
        int_out = out;
        return *this;
    }

    Parser& Parser::AddFloatOutput(double * const out) {
        double_out = out;
        return *this;
    }

    Parser& Parser::AddStringOutput(std::string * const out) {
        string_out = out;
        return *this;
    }

    ErrorCode Parser::CastOutput(const std::string& parsed) {
        ErrorCode ret = GZAT_SUCCESS;
        try {
            if(int_out) {
                *int_out = std::stoi(parsed);
            }
            else if(double_out) {
                *double_out = std::stod(parsed);
            }
            else if(string_out) {
                *string_out = parsed;
                if((*string_out)[0] == '\"') {
                    string_out->erase(0,1);
                }
                if((*string_out)[string_out->size()-1] == '\"') {
                    string_out->erase(string_out->size()-1,1);
                }
            }
            else {
                for(parser_list_t::iterator iter = child_parsers.begin(); 
                    (iter != child_parsers.end()) && (ret == GZAT_SUCCESS); iter++) {
                    ret = (*iter)->Parse(parsed);
                }
            }
        }
        catch(std::exception &e) {
            ret = GZAT_ERROR;
        }
        return ret;
    }

    CommandParser::CommandParser(const AtCommand& cmd) {
        cmd_req = cmd;
    }

    ErrorCode CommandParser::Parse(const std::string& response) {
        ErrorCode err;
        // Find command
        std::cout << response << " " << cmd_req.cmd_id << std::endl;
        size_t s_pos = response.find(cmd_req.cmd_id);
        if(s_pos == std::string::npos) {
            err = GZAT_ERROR;
        }
        else {
            std::cout << response << std::endl;
            err = CastOutput(response.substr(s_pos+cmd_req.cmd_id.size()+1));
        }
        return err;
    }

    CommaSplitParser::CommaSplitParser(const size_t pos) : Parser() {
        pos_in = pos;
    }

    ErrorCode CommaSplitParser::Parse(const std::string& response) {
        ErrorCode err = GZAT_SUCCESS;
        size_t pos_cur = 0U;
        std::string response_cache = response;
        while((pos_cur < pos_in) && (err == GZAT_SUCCESS)) {
            size_t s_pos = response_cache.find(",");
            if(s_pos == std::string::npos) {
                err = GZAT_ERROR;
            }
            else {
                response_cache = response_cache.substr(s_pos+1);
            }
            pos_cur++;
        }

        if(err == GZAT_SUCCESS) {
            size_t e_pos = response_cache.find(",");
            if(e_pos == std::string::npos) {
                e_pos = response_cache.find("\r");
            }

            if(e_pos == std::string::npos) {
                err = CastOutput(response_cache.substr(0));
            }
            else {
                err = CastOutput(response_cache.substr(0, e_pos));
            }
        }
        return err;
    }

    NameValueParser::NameValueParser(const size_t pos) : Parser() {
        pos_in = pos;
    }

    ErrorCode NameValueParser::Parse(const std::string& response) {
        return GZAT_NOT_SUPPORTED;
    }

    ParenthesesParser::ParenthesesParser(const size_t pos) : Parser() {
        pos_in = pos;
    }

    ErrorCode ParenthesesParser::Parse(const std::string& response) {
        ErrorCode err;
        // Find first comma
        size_t s_pos = response.find("(");
        if(s_pos == std::string::npos) {
            err = GZAT_ERROR;
        }
        else {
            size_t e_pos = response.substr(s_pos+1).find(")");
            if(e_pos == std::string::npos) {
                err = GZAT_ERROR;
            }
            else {
                err = CastOutput(response.substr(s_pos+1, e_pos));
            }
        }
        return err;
    }

}