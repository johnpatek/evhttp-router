#include <example.h>

#include <csignal>

#include <iostream>
#include <memory>

static std::unique_ptr<event_base, decltype(&event_base_free)> base{nullptr, &event_base_free};

static void handle_signal(int signum)
{
    std::cerr << "Received signal " << signum << std::endl;
    event_base_loopbreak(base.get());
}

int main(int argc, const char **argv)
{
    int rc;
    base.reset(event_base_new());
    std::unique_ptr<evhttp, decltype(&evhttp_free)> http{evhttp_new(base.get()), &evhttp_free};
    std::cerr << "Starting example server on port 8080..." << std::endl;
    rc = evhttp_bind_socket(http.get(), "0.0.0.0", 8080);
    if (rc == 0)
    {
        std::cerr << "Calling example_start()" << std::endl;
        rc = example_start(http.get());
        if (rc == 0)
        {
            std::cerr << "Press Ctrl+C to stop the server." << std::endl;
            signal(SIGINT, handle_signal);
            signal(SIGTERM, handle_signal);
            event_base_dispatch(base.get());
            std::cerr << "Calling example_stop()" << std::endl;
            example_stop();
        }
    }
    return rc;
}