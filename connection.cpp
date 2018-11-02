#include "connection.hpp"

#include <ctime>
#include <ostream>

#include "utilities/string_utilities.hpp"

#include "def/boost_def.hpp"
#include "application.hpp"

namespace da4qi4
{

Connection::Connection(Tcp::socket socket)
    : _socket(std::move(socket)), _parser(new http_parser)
{
    this->init_parser();
    this->init_parser_setting();
}

Connection::~Connection()
{
    delete _parser;
    this->free_multipart_parser();
}

void Connection::init_parser()
{
    http_parser_init(_parser, HTTP_REQUEST);
    _parser->data = this;
}

void Connection::init_parser_setting()
{
    http_parser_settings_init(&_parser_setting);
    _parser_setting.on_message_begin = &Connection::on_message_begin;
    _parser_setting.on_message_complete = &Connection::on_message_complete;
    _parser_setting.on_headers_complete = &Connection::on_headers_complete;
    _parser_setting.on_header_field = &Connection::on_header_field;
    _parser_setting.on_header_value = &Connection::on_header_value;
    _parser_setting.on_url = &Connection::on_url;
    _parser_setting.on_body = &Connection::on_body;
}

void Connection::StartRead()
{
    this->do_read();
}

void Connection::Stop()
{
    this->do_close();
}

void Connection::StartWrite()
{
    this->do_write();
}

void Connection::StartChunkedWrite()
{
    this->do_write_header_for_chunked();
}

void Connection::update_request_after_header_parsed()
{
    if (!_url.empty())
    {
        _request.ParseUrl(std::move(_url));
    }

    _request.SetFlags(_parser->flags);

    if (_parser->flags &  F_CONTENTLENGTH)
    {
        _request.SetContentLength(_parser->content_length);
    }

    _request.MarkKeepAlive(http_should_keep_alive(_parser));
    _request.MarkUpgrade(_parser->upgrade);
    _request.SetMethod(_parser->method);
    _request.SetVersion(_parser->http_major, _parser->http_minor);
    _response.SetVersion(_parser->http_major, _parser->http_minor);
    try_commit_reading_request_header();
    _request.TransferHeadersToCookies();
    _request.ParseContentType();
}

void Connection::try_commit_reading_request_header()
{
    bool have_a_uncommit_header = _reading_header_part == Connection::header_value_part
                                  && !_reading_header_field.empty();

    if (have_a_uncommit_header)
    {
        _request.AppendHeader(std::move(_reading_header_field), std::move(_reading_header_value));
        _reading_header_part = Connection::header_none_part;
    }
}

bool is_100_continue(Request& req)
{
    auto value = req.TryGetHeader("Expect");
    return (value && Utilities::iEquals(*value, "100-continue"));
}

void Connection::process_100_continue_request()
{
    assert(is_100_continue(_request));
    _response.Continue();
    this->StartWrite();
}

void Connection::try_init_multipart_parser()
{
    assert(_request.IsMultiPart());

    if (!_request.GetMultiPartBoundary().empty()) //boundary is unset on some bad request.
    {
        init_multipart_parser(_request.GetMultiPartBoundary());
    }
}

bool Connection::try_route_application()
{
    _app = AppMgr()->FindByURL(_request.GetUrl().path);
    return _app != nullptr;
}

int Connection::on_headers_complete(http_parser* parser)
{
    Connection* cnt = static_cast<Connection*>(parser->data);
    cnt->update_request_after_header_parsed();

    if (is_100_continue(cnt->_request))
    {
        cnt->process_100_continue_request(); //async write
    }

    if (!cnt->try_route_application())
    {
        std::cerr << "no found app." << std::endl;
        return -1;
    }

    if (cnt->_request.IsMultiPart())
    {
        cnt->try_init_multipart_parser();
    }

    cnt->_read_complete = Connection::read_header_complete;
    return 0;
}

int Connection::on_message_begin(http_parser* parser)
{
    Connection* cnt = static_cast<Connection*>(parser->data);
    cnt->_read_complete = Connection::read_none_complete;
    cnt->_body.clear();

    if (cnt->_request.IsKeepAlive())
    {
        cnt->_request.Reset();
    }

    return 0;
}

int Connection::on_message_complete(http_parser* parser)
{
    Connection* cnt = static_cast<Connection*>(parser->data);
    cnt->_request.SetBody(std::move(cnt->_body));
    cnt->_read_complete = Connection::read_message_complete;
    return 0;
}

int Connection::on_header_field(http_parser* parser, char const* at, size_t length)
{
    Connection* cnt = static_cast<Connection*>(parser->data);
    cnt->try_commit_reading_request_header();

    if (length > 0)
    {
        cnt->_reading_header_part = Connection::header_field_part;
        cnt->_reading_header_field.append(at, length);
    }

    return 0;
}

int Connection::on_header_value(http_parser* parser, char const* at, size_t length)
{
    Connection* cnt = static_cast<Connection*>(parser->data);

    if (cnt->_reading_header_part == Connection::header_field_part
        && !cnt->_reading_header_value.empty())
    {
        cnt->_reading_header_value.clear();
    }

    if (length > 0)
    {
        cnt->_reading_header_value.append(at, length);
    }

    if (cnt->_reading_header_part != Connection::header_value_part)
    {
        cnt->_reading_header_part = Connection::header_value_part;
    }

    return 0;
}


int Connection::on_url(http_parser* parser, char const* at, size_t length)
{
    Connection* cnt = static_cast<Connection*>(parser->data);
    cnt->_url.append(at, length);
    return 0;
}

void Connection::try_fix_multipart_bad_request_without_boundary()
{
    assert(!_mp_parser);

    if (_body.find("--") == 0)
    {
        constexpr int const length_of_boundary_start_flag = 2;
        std::string::size_type endln_pos = _body.find("\r\n", length_of_boundary_start_flag);

        if (endln_pos != std::string::npos)
        {
            std::string boundary = _body.substr(length_of_boundary_start_flag
                                                , endln_pos - length_of_boundary_start_flag);
            std::string::size_type len = boundary.size();

            if (len > 2 && boundary[len - 2] == '-' && boundary[len - 1] == '-')
            {
                len -= 2;
            }

            _request.SetMultiPartBoundary(boundary.c_str(), len);
            init_multipart_parser(_request.GetMultiPartBoundary());
        }
    }
}

Connection::MultpartParseStatus Connection::do_multipart_parse()
{
    assert(_request.IsMultiPart());

    if (!_mp_parser)
    {
        try_fix_multipart_bad_request_without_boundary();
    }

    if (!_mp_parser)
    {
        return mp_cannot_init;
    }

    size_t parsed_bytes = multipart_parser_execute(_mp_parser, _body.c_str(), _body.length());

    if (parsed_bytes != _body.length())
    {
        return mp_parse_fail;
    }

    _body.clear(); //body -> multi parts
    return mp_parsing;
}

int Connection::on_body(http_parser* parser, char const* at, size_t length)
{
    Connection* cnt = static_cast<Connection*>(parser->data);
    cnt->_body.append(at, length);

    if (cnt->_request.IsMultiPart())
    {
        MultpartParseStatus status = cnt->do_multipart_parse();

        switch (status)
        {
            case mp_cannot_init:
            case mp_parsing:
                break;

            case   mp_parse_fail:
                std::cerr << "multipart parse fail." << std::endl;
                return -1;
        }
    }

    return 0;
}

int Connection::on_multipart_data_begin(multipart_parser* parser)
{
    Connection* cnt = static_cast<Connection*>(multipart_parser_get_data(parser));
    cnt->_reading_header_part = header_none_part;
    cnt->_reading_header_field.clear();
    cnt->_reading_header_value.clear();
    cnt->_reading_part_data.clear();
    return 0;
}

int Connection::on_multipart_header_field(multipart_parser* parser, char const* at, size_t length)
{
    Connection* cnt = static_cast<Connection*>(multipart_parser_get_data(parser));

    if (cnt->_reading_header_part == header_value_part && !cnt->_reading_header_field.empty())
    {
        cnt->_reading_part.AppendHeader(std::move(cnt->_reading_header_field)
                                        , std::move(cnt->_reading_header_value));
        cnt->_reading_header_part = header_field_part;
    }

    cnt->_reading_header_field.append(at, length);
    return 0;
}

int Connection::on_multipart_header_value(multipart_parser* parser, char const* at, size_t length)
{
    Connection* cnt = static_cast<Connection*>(multipart_parser_get_data(parser));

    if (cnt->_reading_header_part == Connection::header_field_part
        && !cnt->_reading_header_value.empty())
    {
        cnt->_reading_header_value.clear();
    }

    if (length > 0)
    {
        cnt->_reading_header_value.append(at, length);
    }

    if (cnt->_reading_header_part != Connection::header_value_part)
    {
        cnt->_reading_header_part = Connection::header_value_part;
    }

    return 0;
}

int Connection::on_multipart_headers_complete(multipart_parser* parser)
{
    Connection* cnt = static_cast<Connection*>(multipart_parser_get_data(parser));

    if (cnt->_reading_header_part == header_value_part && !cnt->_reading_header_field.empty())
    {
        cnt->_reading_part.AppendHeader(std::move(cnt->_reading_header_field)
                                        , std::move(cnt->_reading_header_value));
        cnt->_reading_header_part = header_none_part;
    }

    return 0;
}


int Connection::on_multipart_data(multipart_parser* parser, char const* at, size_t length)
{
    Connection* cnt = static_cast<Connection*>(multipart_parser_get_data(parser));
    cnt->_reading_part_data.append(at, length);
    return 0;
}

int Connection::on_multipart_data_end(multipart_parser* parser)
{
    Connection* cnt = static_cast<Connection*>(multipart_parser_get_data(parser));
    cnt->_reading_part.SetData(std::move(cnt->_reading_part_data));
    cnt->_request.AddMultiPart(std::move(cnt->_reading_part));
    return 0;
}

int Connection::on_multipart_body_end(multipart_parser* parser)
{
    Connection* cnt = static_cast<Connection*>(multipart_parser_get_data(parser));
    cnt->free_multipart_parser(will_free_mp_parser);
    return 0;
}

void Connection::init_multipart_parser(std::string const& boundary)
{
    if (!_mp_parser_setting || !_mp_parser)
    {
        if (!_mp_parser_setting)
        {
            _mp_parser_setting = new multipart_parser_settings;
            _mp_parser_setting->on_part_data_begin = &Connection::on_multipart_data_begin;
            _mp_parser_setting->on_part_data = &Connection::on_multipart_data;
            _mp_parser_setting->on_part_data_end = &Connection::on_multipart_data_end;
            _mp_parser_setting->on_header_field = &Connection::on_multipart_header_field;
            _mp_parser_setting->on_header_value = &Connection::on_multipart_header_value;
            _mp_parser_setting->on_headers_complete = &Connection::on_multipart_headers_complete;
            _mp_parser_setting->on_body_end = &Connection::on_multipart_body_end;
        }

        if (_mp_parser)
        {
            multipart_parser_free(_mp_parser);
        }

        int const length_of_boundary_start_flag = 2;
        std::string boundary_with_prefix;
        boundary_with_prefix.reserve(length_of_boundary_start_flag + boundary.size());
        boundary_with_prefix = "--" + boundary;
        _mp_parser = multipart_parser_init(boundary_with_prefix.c_str(), _mp_parser_setting);
        multipart_parser_set_data(_mp_parser, this);
    }
}

void Connection::free_multipart_parser(mp_free_flag flag)
{
    if (flag & will_free_mp_setting) //reuse if created
    {
        delete _mp_parser_setting;
        _mp_parser_setting = nullptr;
    }

    if (_mp_parser && (flag & will_free_mp_parser))
    {
        multipart_parser_free(_mp_parser);
        _mp_parser = nullptr;
    }
}

void Connection::do_close()
{
    _socket.close();
}

void Connection::do_read()
{
    auto self(this->shared_from_this());
    _socket.async_read_some(boost::asio::buffer(_buffer)
                            , [self, this](boost::system::error_code ec, std::size_t bytes_transferred)
    {
        std::cout << std::time(nullptr) << std::endl;

        if (ec)
        {
            return;
        }

        auto parsed = http_parser_execute(_parser, &_parser_setting, _buffer.data(), bytes_transferred);

        if (parsed != bytes_transferred)
        {
            std::cerr << "Error: "
                      << http_errno_description(HTTP_PARSER_ERRNO(_parser))
                      << http_errno_name(HTTP_PARSER_ERRNO(_parser));
            return;
        }

        if (_read_complete != read_message_complete)
        {
            do_read();
            return;
        }

        assert((_app != nullptr) && "MUST HAVE A APPLICATION AFTER MESSAGE READ COMPLETED.");

        if (_request.IsFormUrlEncoded())
        {
            _request.ParseFormUrlEncodedData();
        }
        else if (_request.IsFormData())
        {
            auto const& options = _app->GetUploadFileSaveOptions();
            std::string dir = _app->GetUploadRoot().native();
            _request.TransferMultiPartsToFormData(options, dir);
        }

        auto self = shared_from_this();
        _response.SetCharset(_app->GetDefaultCharset());
        _app->Handle(ContextIMP::Make(self)); //connection -> ctx -> app
    });
}

void Connection::prepare_response_headers_about_connection()
{
    auto v = _request.GetVersion();
    bool keepalive = _request.IsKeepAlive();

    if (v.first < 1 || (v.first == 1 && v.second == 0))
    {
        _response.SetVersion(1, 0);

        if (keepalive)
        {
            _response.MarkKeepAlive();
        }
    }
    else
    {
        _response.SetVersion(1, 1);

        if (!keepalive)
        {
            _response.MarkClose();
        }
    }
}

void Connection::do_write()
{
    prepare_response_headers_about_connection();
    std::ostream os(&_write_buffer);
    os << _response;

    ConnectionPtr self = shared_from_this();
    boost::asio::async_write(_socket
                             , _write_buffer
                             , [self, this](errorcode const & ec, size_t bytes_transferred)
    {
        if (ec)
        {
            return;
        }

        _write_buffer.consume(bytes_transferred);

        if (_request.IsKeepAlive() && _response.IsKeepAlive())
        {
            _response.Reset();
            this->reset();
            this->do_read();
        }
    });
}


void Connection::prepare_response_headers_for_chunked_write()
{
    if (!_response.IsChunked())
    {
        _response.MarkChunked();
    }

    auto v = _response.GetVersion();

    if (v.first < 1 || (v.first == 1 && v.second == 0))
    {
        _response.SetVersion(1, 1);
    }

    if (_response.IsClose())
    {
        _response.MarkKeepAlive();
    }
}

void Connection::do_write_header_for_chunked()
{
    prepare_response_headers_for_chunked_write();

    std::ostream os(&_write_buffer);
    os << _response;

    ConnectionPtr self = shared_from_this();

    boost::asio::async_write(_socket
                             , _write_buffer
                             , [self, this](errorcode const & ec, size_t bytes_transferred)
    {
        if (ec)
        {
            return;
        }

        _write_buffer.consume(bytes_transferred);
        do_write_next_chunked_body(0);
    });
}

void Connection::do_write_next_chunked_body(std::time_t wait_start)
{
    bool is_last = false;
    _current_chunked_body = std::move(_response.PopChunkedBody(is_last));

    ConnectionPtr self = shared_from_this();

    if (_current_chunked_body.empty())
    {
        if (!is_last)
        {
            std::time_t now = std::time(nullptr);

            if (wait_start && (now - wait_start) / CLOCKS_PER_SEC > 5)
            {
                std::cerr << "Too long to Wait for chunked data." << std::endl;
                return;
            }

            _socket.get_io_context().post(std::bind(&Connection::do_write_next_chunked_body, self, now));
        }

        return;
    }

    boost::asio::async_write(_socket, boost::asio::buffer(_current_chunked_body)
                             , std::bind(&Connection::do_write_next_chunked_body, self, 0));
}

void Connection::reset()
{
    _url.clear();
    _body.clear();
    _reading_header_part = header_none_part;
    _reading_header_field.clear();
    _reading_header_value.clear();
    _read_complete = read_none_complete;
    _reading_part.Clear();
    _reading_part_data.clear();
    _app = nullptr;
}

} //namespace da4qi4
