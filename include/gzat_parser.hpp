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

#ifndef __GZAT__PARSER_HPP_
#define __GZAT__PARSER_HPP_
#include <string>
#include <vector>
#include <memory>
namespace gzat {

    const std::string MS_LUT[7] =
    {
        "",             // Default - skipped by search
        "+",            // AT+...
        "#",            // AT#...
        "$",            // AT$...
        "%",            // AT%...
        "\\",           // AT\...
        "&"             // AT&...
    };

    const std::string ME_LUT[6] =
    {
        "",             // Default - skipped by search
        "=?",           // Test
        "?",            // Get
        "=",            // Set
        ":",            // URC 100%
        "\r"            // Exec
    };

    /**
     * AT Command structure
     */
    struct AtCommand {
        uint8_t ms;     ///< Index of the start marker
        uint8_t me;     ///< Index of the end marker
        std::string cmd_id;         ///< Command ID
        std::string cmd_payload;    ///< Command Payload

        /**
         * Default constructor
         */
        AtCommand();

        /**
         * Converts raw AT command to struct
         * @param[in] raw_cmd       Raw AT command
         */
        AtCommand(const std::string & raw_cmd);

        /**
         * Generate raw AT command
         * @return                  Raw AT command
         */
        std::string GetRawCommand() const;
    };

    enum ErrorCode {
        GZAT_SUCCESS,               ///< Action succeeded
        GZAT_ERROR,                 ///< Action errored
        GZAT_NOT_SUPPORTED          ///< Action not supported
    };

    /**
     * This class defines a base parser class for parsing AT command response.
     */
    class Parser {
    public:
        /**
         * Default constructor
         */
        Parser();

        /**
         * Parse repsones
         * @param[in]   response        Response of a command
         * @return      ErrorCode
         */
        virtual ErrorCode Parse(const std::string& response) = 0;

        /**
         * Add a child parser.
         * Children parsers will be executed in order after this parser is executed
         * @param[in]   parser          A child parser to add
         * @return      this parser
         */
        Parser& AddChildParser(const std::shared_ptr<Parser> parser);
        
        /**
         * Specify a possible integer output from this parser
         * @param[out]  out             Parsed integer value
         * @return      this parser
         */
        Parser& AddIntegerOutput(int64_t * const out);

        /**
         * Specify a possible float output from this parser
         * @param[out]  out             Parsed float value
         * @return      this parser
         */
        Parser& AddFloatOutput(double * const out);

        /**
         * Specify a possible string output from this parser
         * @param[out]  out             Parsed string value
         * @return      this parser
         */
        Parser& AddStringOutput(std::string * const out);

    protected:
        ErrorCode CastOutput(const std::string& parsed);

        int64_t * int_out;
        double * double_out;
        std::string * string_out;
        size_t pos_in;
        typedef std::vector<std::shared_ptr<Parser> > parser_list_t;
        parser_list_t child_parsers;
    };

    /**
     * This class defines a parser class for parsing AT command response
     * that has command echoed at front of response
     */
    class CommandParser : public Parser {
    public:
        /**
         * Constructor with given command
         * @param[in]   cmd             Command sent originally for which response will be parsed
         */
        CommandParser(const AtCommand& cmd);

        /**
         * Parse repsones
         * @param[in]   response        Response of a command
         * @return      ErrorCode
         */
        virtual ErrorCode Parse(const std::string& response);
    protected:
        AtCommand cmd_req;
    };

    /**
     * This class defines a parser class for parsing AT command response
     * that has payloads splitted by commas
     * E.g., <a>,<b>
     */
    class CommaSplitParser : public Parser {
    public:
        /**
         * Constructor with given position
         * @param[in]   pos             Position of the field among comma splitted fields
         */
        CommaSplitParser(const size_t pos);

        /**
         * Parse repsones
         * @param[in]   response        Response of a command
         * @return      ErrorCode
         */
        virtual ErrorCode Parse(const std::string& response);
    };

    /**
     * This class defines a parser class for parsing AT command response
     * that has payloads splitted as name value pairs
     * E.g., <a>:<b> <c>:<d>
     */
    class NameValueParser : public Parser {
    public:
        /**
         * Constructor with given position
         * @param[in]   pos             Position of the field among name value pairs
         */
        NameValueParser(const size_t pos);

        /**
         * Parse repsones
         * @param[in]   response        Response of a command
         * @return      ErrorCode
         */
        virtual ErrorCode Parse(const std::string& response);
    };

    /**
     * This class defines a parser class for parsing AT command response
     * that has payloads splitted by parentheses
     * E.g., (a)(b)
     */
    class ParenthesesParser : public Parser {
    public:
        /**
         * Constructor with given position
         * @param[in]   pos             Position of the field among parentheses splitted fields
         */
        ParenthesesParser(const size_t pos);

        /**
         * Parse repsones
         * @param[in]   response        Response of a command
         * @return      ErrorCode
         */
        virtual ErrorCode Parse(const std::string& response);
    };

}

#endif //__GZAT__PARSER_HPP_