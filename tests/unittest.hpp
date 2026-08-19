#ifndef UNITTEST_HPP
#define UNITTEST_HPP

#include <evhttp_router.hpp>

#include <event2/event.h>

#include <maxtest.hpp>

#include <atomic>
#include <functional>

namespace unittest
{
    class TestClient
    {
    private:
        event_base *_base;
        std::unique_ptr<evhttp_connection, decltype(&evhttp_connection_free)> _connection{nullptr, &evhttp_connection_free};
        std::atomic<int> _counter;
        struct RequestData
        {
            std::atomic<int> *counter;
            event_base *base;
            std::function<void(evhttp_request*)> callback;
        };
        static void requestDone(evhttp_request *req, void *arg);
    public:
        template<int Code>
        static void expectCode(evhttp_request *req)
        {
            const int code = evhttp_request_get_response_code(req);
            MAXTEST_ASSERT(code == Code);
        }
        TestClient(event_base *base);
        ~TestClient() = default;
        void makeRequest(evhttp_cmd_type cmd, const char * path, const std::function<void(evhttp_request*)>& callback);
        bool hasRequests();
    };

    class TestSuite
    {
    private:
        std::unique_ptr<event_base, decltype(&event_base_free)> _base {event_base_new(), &event_base_free};
        std::unique_ptr<evhttp, decltype(&evhttp_free)> _http = {nullptr, evhttp_free};
        std::unique_ptr<TestClient> _client;
    protected:
        virtual void setup(evhttp *http, TestClient& client) = 0;    
    public:
        void run();
    };

    class TestCase : public TestSuite
    {
    private:
        std::unique_ptr<evhttp_router, decltype(&evhttp_router_free)> _router{nullptr, &evhttp_router_free};
    protected:
        void setup(evhttp *http, TestClient& client) override;
        virtual void onSetup(evhttp_router *router, TestClient& client) = 0;
    };

    class TestCaseCpp : public TestSuite
    {
    private:    
        std::unique_ptr<evhttprouter::Router> _router{nullptr};
    protected:
        void setup(evhttp *http, TestClient& client) override;
        virtual void onSetup(evhttprouter::Router &router, TestClient& client) = 0;
    };
}

#endif