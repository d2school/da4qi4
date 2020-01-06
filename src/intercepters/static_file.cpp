#include "daqi/intercepters/static_file.hpp"

#include <fstream>

#include "daqi/application.hpp"
#include "daqi/utilities/string_utilities.hpp"
#include "daqi/utilities/html_utilities.hpp"

namespace da4qi4
{
namespace Intercepter
{

std::string const StaticFile::data_name = "static-file";

StaticFile& StaticFile::AddEntry(std::string const& url_root
                                 , std::string const& dir_root)
{
    _root_entries.insert(std::make_pair(url_root, (dir_root.empty() ? url_root : dir_root)));
    return *this;
}

StaticFile& StaticFile::AddDefaultFileName(std::string const& index_filename)
{
    for (auto fn : _default_filenames)
    {
        if (fn == index_filename)
        {
            return *this;
        }
    }

    _default_filenames.push_back(index_filename);

    return *this;
}

StaticFile& StaticFile::AddDefaultFileNames(std::vector<std::string> const&
                                            index_filenames)
{
    for (auto s : index_filenames)
    {
        AddDefaultFileName(s);
    }

    return *this;
}

void StaticFile::on_request(Context& ctx) const
{
    HandlerMethod m = from_http_method(ctx->Req().GetMethod());

    if ((m != HandlerMethod::GET))
    {
        ctx->Pass();
        return;
    }

    std::string url = ctx->Req().GetUrl().full;

    bool entry_found = false;
    fs::path dst_file;

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

            std::string::size_type beg_pos = url_starts.size();
            std::string::size_type end_pos = (url.find('?', beg_pos));
            std::string::size_type url_size = url.size();
            std::string::size_type sub_len = url_size - beg_pos
                                             - (end_pos != std::string::npos
                                                ? (url_size - end_pos) : 0);

            dst_file = dir_root;
            dst_file /= url.substr(beg_pos, sub_len);
            entry_found = true;
            break;
        }
    }

    if (entry_found)
    {
        Json status_data =
        {
            {"found", entry_found},
            {"file", dst_file.string()}
        };

        ctx->SaveData(data_name, std::move(status_data));
        ctx->Stop();
        return;
    }

    ctx->Pass();
}

void StaticFile::on_response(Context& ctx) const
{
    Json status_data = ctx->LoadData(data_name);

    if (status_data.empty())
    {
        ctx->Pass();
        return;
    }

    auto it = status_data.find("found");
    bool entry_found = (it == status_data.end() ? false : it->get<bool>());

    if (!entry_found)
    {
        ctx->Pass();
        return;
    }

    it = status_data.find("file");
    std::string dst_file_name = (it == status_data.end() ? "" : *it);

    if (dst_file_name.empty())
    {
        ctx->RenderBadRequest();
        return;
    }

    fs::path dst_file(dst_file_name);

    try
    {
        bool file_exists = dst_file.has_filename()
                           && dst_file.filename() != "." && fs::exists(dst_file);

        if (!file_exists)
        {
            bool found = false;

            if (!dst_file.has_filename() || dst_file.filename() == ".")
            {
                for (auto fn : _default_filenames)
                {
                    fs::path with_default_file = dst_file / fn;

                    if (fs::exists(with_default_file))
                    {
                        found = true;
                        dst_file = with_default_file;
                        break;
                    }
                }
            }

            if (!found)
            {
                ctx->RenderNofound();
                ctx->Pass();
                return;
            }
        }
    }
    catch (std::exception const& e)
    {
        ctx->Logger()->error("Locate static file {} exception. {}", dst_file_name, e.what());

        ctx->RenderInternalServerError();
        ctx->Pass();
        return;
    }

    std::ifstream ifs(dst_file.native().c_str(), std::ios_base::binary);

    if (!ifs)
    {
        ctx->Logger()->error("Open {} file fail.", dst_file_name);

        ctx->RenderInternalServerError();
        ctx->Pass();
        return;
    }

    size_t const max_byte_read_one_time = 1024 * 10;
    char rdbuf[max_byte_read_one_time];

    size_t const max_byte_one_chunked_body = 1024 * 100;
    std::string a_chunk_body;
    a_chunk_body.reserve(max_byte_one_chunked_body);

    std::string content_type = Utilities::GetMIMEType(dst_file.extension().string());

    if (!content_type.empty())
    {
        ctx->Res().SetContentType(content_type);
    }

    ctx->Res().CacheControlMaxAge(_cache_max_age);
    ctx->StartChunkedResponse();

    while (ifs)
    {
        ifs.read(rdbuf, max_byte_read_one_time);
        auto count = ifs.gcount();

        if (count > 0)
        {
            a_chunk_body.append(rdbuf, rdbuf + count);
        }

        if (a_chunk_body.size() > max_byte_one_chunked_body)
        {
            ctx->NextChunkedResponse(a_chunk_body);
            a_chunk_body.clear();
        }
    }

    if (!a_chunk_body.empty())
    {
        ctx->NextChunkedResponse(a_chunk_body);
        a_chunk_body.clear();
    }

    ctx->RemoveData(data_name);
    ctx->StopChunkedResponse();
    ctx->Pass();
}

void StaticFile::operator()(Context ctx, On on) const
{
    if (on == Intercepter::On::Request)
    {
        on_request(ctx);
    }
    else
    {
        on_response(ctx);
    }
}

} //namespace Intercepter
} //namespace da4qi4
