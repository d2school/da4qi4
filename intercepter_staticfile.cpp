#include "intercepter_staticfile.hpp"

#include <fstream>

#include "application.hpp"
#include "utilities/string_utilities.hpp"
#include "utilities/html_utilities.hpp"

namespace da4qi4
{
namespace Intercepter
{

StaticFileIntercepter& StaticFileIntercepter::AddEntry(std::string const& url_root
                                                       , std::string const& dir_root)
{
    _root_entries.insert(std::make_pair(url_root, dir_root));
    return *this;
}

void StaticFileIntercepter::operator()(Context ctx) const
{
    HandlerMethod m = from_http_method((http_method)ctx->Req().GetMethod());

    if ((m != HandlerMethod::GET))
    {
        Pass(ctx);
        return;
    }

    std::string url = ctx->Req().GetUrl().full; //absolute
    fs::path dst_path_filename;

    bool entry_found = false;

    for (auto const& entry : _root_entries)
    {
        std::string url_starts = ((_url_resolve_type == PathResolve::is_relative)
                                  ? ctx->App().GetUrlRoot() + entry.first
                                  : entry.first);

        if (Utilities::iStartsWith(url, url_starts))
        {
            std::string dir_root = ((_dir_resolve_type == PathResolve::is_relative)
                                    ? ctx->App().GetStaticRootPath().native() + entry.second
                                    : entry.second);

            dst_path_filename = dir_root;
            dst_path_filename /= url.substr(url_starts.size());
            entry_found = true;
            break;
        }
    }

    if (!entry_found)
    {
        Pass(ctx);
        return;
    }

    if (dst_path_filename.empty())
    {
        ctx->RenderNofound();
        StopOnError(ctx);
        return;
    }

    errorcode ec;

    if (!fs::exists(dst_path_filename, ec))
    {
        std::cerr << ec.message() << std::endl;

        ctx->RenderNofound();
        StopOnError(ctx);
        return;
    }


    std::ifstream ifs(dst_path_filename.native().c_str(), std::ios_base::binary);

    if (!ifs)
    {
        std::cerr << "Open " << dst_path_filename.native() << "file fail!" << std::endl;

        ctx->RenderInternalServerError();
        StopOnError(ctx);
        return;
    }

    size_t const max_size_read_one_time = 1024 * 2;
    char rdbuf[max_size_read_one_time];

    size_t const max_size_one_chunked_body = 1024 * 128;
    std::string a_chunk_body;
    a_chunk_body.reserve(max_size_one_chunked_body);

    std::string content_type = Utilities::GetMIMEType(dst_path_filename.extension().string());
    ctx->Res().SetContentType(content_type);
    ctx->Res().SetPublicCache(_max_age);

    ctx->StartChunkedResponse();

    while (ifs)
    {
        ifs.read(rdbuf, max_size_read_one_time);
        size_t count = ifs.gcount();

        if (count > 0)
        {
            a_chunk_body.append(rdbuf, rdbuf + count);
        }

        if (a_chunk_body.size() > max_size_one_chunked_body)
        {
            ctx->ContinueChunkedResponse(a_chunk_body);
            a_chunk_body.clear();
        }
    }

    if (!a_chunk_body.empty())
    {
        ctx->ContinueChunkedResponse(a_chunk_body);
        a_chunk_body.clear();
    }

    StopOnSuccess(ctx);
    ctx->Bye();
}


} //namespace Intercepter
} //namespace da4qi4
