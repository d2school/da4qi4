#include <iostream>

#include "asio_def.hpp"

#include "server.hpp"

int main()
{
    boost::asio::io_context ioc;
    try
    {
        da4qi4::Server svc(ioc, 4099);
        svc.Start();
        ioc.run();
    }
    catch(std::exception const& e)
    {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}
