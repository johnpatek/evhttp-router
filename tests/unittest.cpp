#include <unittest.hpp>

void unittest::TestClient::requestDone(evhttp_request *req, void *arg)
{
    unittest::TestClient::RequestData *data = static_cast<unittest::TestClient::RequestData *>(arg);
    (data->callback)(req);
    data->counter->fetch_sub(1);
    if (data->counter->load() == 0)
    {
        event_base_loopbreak(data->base);
    }
    delete data;
}

bool unittest::TestClient::hasRequests()
{
    return _counter.load() > 0;
}

unittest::TestClient::TestClient(event_base *base) : _base(base)
{
    _connection.reset(evhttp_connection_base_new(base, nullptr, "127.0.0.1", 12345));
}

void unittest::TestClient::makeRequest(evhttp_cmd_type cmd, const char * path, const std::function<void(evhttp_request*)>& callback)
{
    _counter++;
    const std::string uri = "http://127.0.0.1:12345" + std::string(path);
    unittest::TestClient::RequestData *data = new unittest::TestClient::RequestData;
    data->base = _base;
    data->counter = &_counter;
    data->callback = callback;
    evhttp_request *req = evhttp_request_new(unittest::TestClient::requestDone, data);
    evkeyvalq *headers = evhttp_request_get_output_headers(req);
    evhttp_add_header(headers, "Connection", "close");
    evhttp_make_request(_connection.get(), req, cmd, uri.c_str());
}

void unittest::TestSuite::run()
{
    _http.reset(evhttp_new(_base.get()));
    evhttp_set_allowed_methods(_http.get(), EVHTTP_REQ_GET | EVHTTP_REQ_POST | EVHTTP_REQ_PUT | EVHTTP_REQ_DELETE | EVHTTP_REQ_HEAD | EVHTTP_REQ_OPTIONS | EVHTTP_REQ_TRACE | EVHTTP_REQ_PATCH);
    evhttp_bind_socket(_http.get(), "0.0.0.0", 12345);
    _client.reset(new unittest::TestClient(_base.get()));
    setup(_http.get(), *_client);
    if (_client && _client->hasRequests())
    {
        event_base_dispatch(_base.get());
    }
}

void unittest::TestCase::setup(evhttp *http, unittest::TestClient &client)
{
    _router.reset(evhttp_router_new(http));
    onSetup(_router.get(), client);
}

void unittest::TestCaseCpp::setup(evhttp *http, unittest::TestClient &client)
{
    _router.reset(new evhttprouter::Router(http));
    onSetup(*_router, client);
}