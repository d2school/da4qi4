#include "connection.hpp"

#include <iostream>

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

void Connection::Start()
{
    this->do_read();
}

void Connection::Stop()
{
    this->do_close();
}

int Connection::on_headers_complete(http_parser* parser)
{
    Connection* cnt = static_cast<Connection*>(parser->data);

    if (!cnt->_url.empty())
    {
        cnt->_request.ParseUrl(std::move(cnt->_url));
    }

    cnt->_request.SetFlags(parser->flags);
    if (parser->flags &  F_CONTENTLENGTH)
    {
        cnt->_request.SetContentLength(parser->content_length);
    }

    cnt->_request.MarkKeepAlive(http_should_keep_alive(parser));
    cnt->_request.MarkUpgrade(parser->upgrade);

    cnt->_request.SetMethod(parser->method);
    cnt->_request.SetVersion(parser->http_major, parser->http_minor);
    

    if (cnt->_reading_header_part != Connection::header_none_part
            && !cnt->_reading_header_field.empty())
    {
        cnt->_request.AppendHeader(std::move(cnt->_reading_header_field)
                                   , std::move(cnt->_reading_header_value));
        cnt->_reading_header_part = Connection::header_none_part;
    }

    cnt->_request.ParseContentType();
    
    if (cnt->_request.IsMultiPart())
    {
        if (!cnt->_request.GetMultiPartBoundary().empty())
        {
            cnt->init_multipart_parser(cnt->_request.GetMultiPartBoundary());
        }
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

    if (cnt->_reading_header_part == Connection::header_value_part
             && !cnt->_reading_header_field.empty())
    {
        cnt->_request.AppendHeader(std::move(cnt->_reading_header_field)
                                   , std::move(cnt->_reading_header_value));
        cnt->_reading_header_part = Connection::header_none_part;
    }

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
        cnt->_reading_header_part = Connection::header_none_part;
    }

    if (length > 0)
    {
        cnt->_reading_header_part = Connection::header_value_part;
        cnt->_reading_header_value.append(at, length);
    }

    return 0;
}


int Connection::on_url(http_parser* parser, char const* at, size_t length)
{
    Connection* cnt = static_cast<Connection*>(parser->data);
    cnt->_url.append(at, length);

    return 0;
}

int Connection::on_body(http_parser* parser, char const* at, size_t length)
{
    Connection* cnt = static_cast<Connection*>(parser->data);
    cnt->_body.append(at, length);
    
    std::cout << cnt->_body << "\r\n========================\r\n" << std::endl;
    
    if (cnt->_request.IsMultiPart())
    {
        if (!cnt->_mp_parser && cnt->_body.find("--") == 0)
        {
            constexpr int const length_of_boundary_start_flag = 2;
            std::string::size_type endln_pos = cnt->_body.find("\r\n", length_of_boundary_start_flag);

            if (endln_pos != std::string::npos)
            {
                std::string boundary = cnt->_body.substr(length_of_boundary_start_flag
                                                         , endln_pos - length_of_boundary_start_flag);
                std::string::size_type len = boundary.size();                    
                if (len > 2 && boundary[len-2] == '-' && boundary[len-1] == '-')
                {
                    len -= 2;
                }
                
                cnt->_request.SetMultiPartBoundary(boundary.c_str(), len);
                cnt->init_multipart_parser(cnt->_request.GetMultiPartBoundary());                                        
            }            
        }
        
        if (cnt->_mp_parser) 
        {
            size_t parsed_bytes = multipart_parser_execute(cnt->_mp_parser, cnt->_body.c_str(), cnt->_body.length());
            std::cout << "multipart parser parsed : " << parsed_bytes << ", src length : " << cnt->_body.length() << "." << std::endl;
            cnt->_body.clear();
        }
    }

    return 0;
}

int Connection::on_multipart_header_field(multipart_parser* parser, char const* at, size_t length)
{
    Connection* cnt = static_cast<Connection *> (multipart_parser_get_data(parser));
    
    std::cout << "multipart header field : " << std::string(at, length) << std::endl;
    return 0;
}

int Connection::on_multipart_header_value(multipart_parser* parser, char const* at, size_t length)
{
    Connection* cnt = static_cast<Connection *> (multipart_parser_get_data(parser));

    std::cout << "multipart header value : " << std::string(at, length) << std::endl;
    
    return 0;
}

int Connection::on_multipart_headers_complete(multipart_parser* parser)
{
    Connection* cnt = static_cast<Connection *> (multipart_parser_get_data(parser));
    std::cout << "on_multipart_headers_complete ()"  << std::endl;
    return 0;
}

int Connection::on_multipart_data_begin(multipart_parser* parser)
{
    Connection* cnt = static_cast<Connection *> (multipart_parser_get_data(parser));
    std::cout << "on_multipart_data_begin ()"  << std::endl;
    return 0;
}

int Connection::on_multipart_data(multipart_parser* parser, char const* at, size_t length)
{
    Connection* cnt = static_cast<Connection *> (multipart_parser_get_data(parser));
    std::cout << "multipart_data : " << std::string(at, length) << std::endl;    
    return 0;        
}

int Connection::on_multipart_data_end(multipart_parser* parser)
{
    Connection* cnt = static_cast<Connection *> (multipart_parser_get_data(parser));
    std::cout << "on_multipart_data_end ()"  << std::endl;
    return 0;    
}

int Connection::on_multipart_body_end(multipart_parser* parser)
{
    Connection* cnt = static_cast<Connection *> (multipart_parser_get_data(parser));
    std::cout << "on_multipart_body_end ()"  << std::endl;
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
            _mp_parser_setting->on_headers_complete =&Connection::on_multipart_headers_complete;
            _mp_parser_setting->on_body_end = &Connection::on_multipart_body_end;                
        }
        
        if (_mp_parser)
        {
            multipart_parser_free(_mp_parser);
        }

        constexpr int const length_of_boundary_start_flag = 2;
        std::string boundary_with_prefix;
        boundary_with_prefix.reserve(length_of_boundary_start_flag + boundary.size());
        boundary_with_prefix = "--" + boundary;
        
        _mp_parser = multipart_parser_init(boundary_with_prefix.c_str(), _mp_parser_setting);
        multipart_parser_set_data(_mp_parser, this);
    }
}

void Connection::free_multipart_parser()
{
    if(_mp_parser_setting)
    {
        delete _mp_parser_setting;
        _mp_parser_setting = nullptr;
    }

    if (_mp_parser)
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
    auto self (this->shared_from_this());

    _socket.async_read_some(boost::asio::buffer(_buffer)
            , [self, this](boost::system::error_code ec, std::size_t bytes_transferred)
    {
        if (!ec)
        {
            int parsed = http_parser_execute(_parser, &_parser_setting, _buffer.data(), bytes_transferred);

            if (parsed != bytes_transferred)
            {
                std::cerr << "Error: "
                           << http_errno_description(HTTP_PARSER_ERRNO(_parser))
                           << http_errno_name(HTTP_PARSER_ERRNO(_parser));
                return;
            }

            if (_read_complete == read_message_complete)
            {
                _request.Dump();
            }

            do_read();
        }
    });
}

} //namespace da4qi4
