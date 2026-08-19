#include <unittest.hpp>

class BasicTest : public unittest::TestCase
{
private:
    static void basicCallback(evhttp_request *req, const struct evhttp_pathvars *vars, void *arg)
    {
        evhttp_send_reply(req, HTTP_OK, "Ok", nullptr);
    }

protected:
    void onSetup(evhttp_router *router, unittest::TestClient &client) override
    {
        const evhttp_handler handler = {
            .get_cb = basicCallback};
        evhttp_router_handle(router, "/resource", &handler, nullptr);
        client.makeRequest(EVHTTP_REQ_GET, "/resource", unittest::TestClient::expectCode<HTTP_OK>);
        client.makeRequest(EVHTTP_REQ_GET, "/invalid", unittest::TestClient::expectCode<HTTP_NOTFOUND>);
    }

public:
    BasicTest() = default;
    ~BasicTest() = default;
};

class MethodTestData
{
private:
    using EmptyHandler = evhttprouter::Handler;

    class OkHandler : public evhttprouter::Handler
    {
    private:
        static void sendOk(evhttp_request *req)
        {
            evhttp_send_reply(req, HTTP_OK, "Ok", nullptr);
        }
    public:
        static void handle(evhttp_request *req, const evhttp_pathvars *vars, void *arg)
        {
            sendOk(req);
        }
        void onGet(evhttp_request *req, const evhttprouter::PathVars &vars) override { sendOk(req); }
        void onPost(evhttp_request *req, const evhttprouter::PathVars &vars) override { sendOk(req); }
        void onPut(evhttp_request *req, const evhttprouter::PathVars &vars) override { sendOk(req); }
        void onDelete(evhttp_request *req, const evhttprouter::PathVars &vars) override { sendOk(req); }
        void onHead(evhttp_request *req, const evhttprouter::PathVars &vars) override { sendOk(req); }
        void onOptions(evhttp_request *req, const evhttprouter::PathVars &vars) override { sendOk(req); }
        void onTrace(evhttp_request *req, const evhttprouter::PathVars &vars) override { sendOk(req); }
        void onPatch(evhttp_request *req, const evhttprouter::PathVars &vars) override { sendOk(req); }
    };
    EmptyHandler _emptyHandler;
    OkHandler _okHandler;

public:
    MethodTestData() = default;
    ~MethodTestData() = default;

    void setupRouter(evhttp_router *router)
    {
        const evhttp_handler okHandler = {
            .get_cb = OkHandler::handle,
            .post_cb = OkHandler::handle,
            .head_cb = OkHandler::handle,
            .put_cb = OkHandler::handle,
            .delete_cb = OkHandler::handle,
            .options_cb = OkHandler::handle,
            .trace_cb = OkHandler::handle,
            .patch_cb = OkHandler::handle,
        };
        const evhttp_handler emptyHandler = {
            .get_cb = nullptr,
            .post_cb = nullptr,
            .head_cb = nullptr,
            .put_cb = nullptr,
            .delete_cb = nullptr,
            .options_cb = nullptr,
            .trace_cb = nullptr,
            .patch_cb = nullptr,
        };
        evhttp_router_handle(router, "/ok", &okHandler, nullptr);
        evhttp_router_handle(router, "/empty", &emptyHandler, nullptr);
    }

    void setupRouter(evhttprouter::Router &router)
    {
        router.handle("/ok", &_okHandler);
        router.handle("/empty", &_emptyHandler);
    }

    void setupClient(unittest::TestClient &client)
    {
        const evhttp_cmd_type methods[8] = {
            EVHTTP_REQ_GET,
            EVHTTP_REQ_POST,
            EVHTTP_REQ_PUT,
            EVHTTP_REQ_DELETE,
            EVHTTP_REQ_HEAD,
            EVHTTP_REQ_TRACE,
            EVHTTP_REQ_PATCH,
            EVHTTP_REQ_OPTIONS,
        };
        for (const auto &method : methods)
        {
            client.makeRequest(method, "/ok", unittest::TestClient::expectCode<HTTP_OK>);
            client.makeRequest(method, "/empty", unittest::TestClient::expectCode<HTTP_BADMETHOD>);
        }
        client.makeRequest(static_cast<evhttp_cmd_type>(3), "/ok", unittest::TestClient::expectCode<HTTP_NOTIMPLEMENTED>);
    }
};

class MethodTest : public unittest::TestCase, public MethodTestData
{
protected:
    virtual void onSetup(evhttp_router *router, unittest::TestClient &client)
    {
        setupRouter(router);
        setupClient(client);
    }
};

class MethodTestCpp : public unittest::TestCaseCpp, public MethodTestData
{
protected:
    virtual void onSetup(evhttprouter::Router &router, unittest::TestClient &client)
    {
        setupRouter(router);
        setupClient(client);
    }
};

class WildcardTestData
{
private:
    template<int ParameterCount>
    static void handleGetWildcard(evhttp_request *req, const evhttp_pathvars *vars, void *arg)
    {
        MAXTEST_ASSERT(evhttp_pathvars_size(vars) == ParameterCount);
        bool wildcardMatch(true);
        for (int index = 0; index < ParameterCount; ++index)
        {
            std::string_view value(evhttp_pathvars_get(vars, index));
            wildcardMatch &= (value == std::string_view("valid"));
        }
        if (wildcardMatch)
        {
            evhttp_send_reply(req, HTTP_OK, "Ok", nullptr);
        }
        else
        {
            evhttp_send_error(req, HTTP_NOTFOUND, "Not Found");
        }
    }
public:
    void setupRouter(evhttp_router *router)
    {
        const evhttp_handler singleHandler = {
            .get_cb = &handleGetWildcard<1>,
        };
        const evhttp_handler multiHandler = {
            .get_cb = &handleGetWildcard<2>,
        };
        evhttp_router_handle(router, "/single/*/wildcard", &singleHandler, nullptr);
        evhttp_router_handle(router, "/multi/*/wildcard/*/handler", &multiHandler, nullptr);
    }

    void setupClient(unittest::TestClient &client)
    {
        client.makeRequest(EVHTTP_REQ_GET, "/single/valid/wildcard", unittest::TestClient::expectCode<HTTP_OK>);
        client.makeRequest(EVHTTP_REQ_GET, "/single/invalid/wildcard", unittest::TestClient::expectCode<HTTP_NOTFOUND>);
        client.makeRequest(EVHTTP_REQ_GET, "/multi/valid/wildcard/valid/handler", unittest::TestClient::expectCode<HTTP_OK>);
        client.makeRequest(EVHTTP_REQ_GET, "/multi/invalid/wildcard/valid/handler", unittest::TestClient::expectCode<HTTP_NOTFOUND>);
        client.makeRequest(EVHTTP_REQ_GET, "/multi/valid/wildcard/invalid/handler", unittest::TestClient::expectCode<HTTP_NOTFOUND>);
    }
};

class WildcardTest : public unittest::TestCase, public WildcardTestData
{
protected:
    virtual void onSetup(evhttp_router *router, unittest::TestClient &client)
    {
        setupRouter(router);
        setupClient(client);
    }
};

class HandlerTest : public unittest::TestCase
{
private:
    static void handleGet(evhttp_request *req, const struct evhttp_pathvars *vars, void *arg)
    {
        evhttp_send_reply(req, HTTP_OK, "Ok", nullptr);
    }
protected:
    virtual void onSetup(evhttp_router *router, unittest::TestClient &client)
    {
        const evhttp_handler handler = {
            .get_cb = &handleGet,
        };
        evhttp_router_handle(router, "/removed", &handler, nullptr);
        evhttp_router_handle(router, "/removed", nullptr, nullptr);
        client.makeRequest(EVHTTP_REQ_GET, "/removed", unittest::TestClient::expectCode<HTTP_NOTFOUND>);
    }
};

#define TEST_CASE(TestClass, name) \
    MAXTEST_TEST_CASE(name)        \
    {                              \
        TestClass test;            \
        test.run();                \
    };

MAXTEST_MAIN
{
    TEST_CASE(BasicTest, basic_test)
    TEST_CASE(MethodTest, method_test)
    TEST_CASE(MethodTestCpp, method_test_cpp)
    TEST_CASE(WildcardTest, wildcard_test)
    TEST_CASE(HandlerTest, handler_test)
};