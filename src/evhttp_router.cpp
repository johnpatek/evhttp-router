#include <evhttp_router.h>

#include <algorithm>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include <cstring>

class evhttp_pathvars
{
private:
    const std::vector<std::string> &_vars;

public:
    evhttp_pathvars(const std::vector<std::string> &vars) : _vars(vars) {}
    ~evhttp_pathvars() = default;
    const char *get(int index) const
    {
        return _vars.at(index).c_str();
    }

    int size() const
    {
        return static_cast<int>(_vars.size());
    }
};

class evhttp_router
{
private:
    std::vector<std::pair<std::string, std::unique_ptr<evhttp_router>>> _child_nodes;
    std::unique_ptr<evhttp_router> _wildcard_node;
    std::unique_ptr<evhttp_handler> _handler;
    void *_arg;

public:
    evhttp_router() = default;
    ~evhttp_router() = default;

    evhttp_router *get_child(const std::string &segment, bool create_if_missing)
    {
        evhttp_router *result(nullptr);
        auto it = std::find_if(_child_nodes.begin(), _child_nodes.end(),
                               [&segment](const auto &pair)
                               { return pair.first == segment; });
        if (it == _child_nodes.end())
        {
            if (create_if_missing)
            {
                auto new_node = std::make_unique<evhttp_router>();
                result = new_node.get();
                _child_nodes.emplace_back(segment, std::move(new_node));
            }
        }
        else
        {
            result = it->second.get();
        }
        return result;
    }

    evhttp_router *get_wildcard(bool create_if_missing)
    {
        evhttp_router *result(nullptr);
        if (_wildcard_node == nullptr && create_if_missing)
        {
            _wildcard_node = std::make_unique<evhttp_router>();
        }
        result = _wildcard_node.get();
        return result;
    }

    std::pair<evhttp_handler *, void *> get_handler()
    {
        return {_handler.get(), _arg};
    }

    void set_handler(const evhttp_handler *handler, void *arg)
    {
        evhttp_handler *new_handler(nullptr);
        if (handler != nullptr)
        {
            new_handler = new evhttp_handler;
            new_handler->get_cb = handler->get_cb;
            new_handler->post_cb = handler->post_cb;
            new_handler->head_cb = handler->head_cb;
            new_handler->put_cb = handler->put_cb;
            new_handler->delete_cb = handler->delete_cb;
            new_handler->options_cb = handler->options_cb;
            new_handler->trace_cb = handler->trace_cb;
            new_handler->patch_cb = handler->patch_cb;
        }
        _handler.reset(new_handler);
        _arg = arg;
    }
};

template <typename SegmentHandler>
evhttp_router *evhttp_route(evhttp_router *router, const char *path, SegmentHandler &segment_handler)
{
    evhttp_router *current = router;
    std::string_view path_view(path);
    size_t start(0);
    size_t end;
    std::string_view segment;
    while (start != std::string_view::npos && !path_view.empty() && current != nullptr)
    {
        start = path_view.find_first_not_of('/');
        if (start != std::string_view::npos)
        {
            end = path_view.find_first_of('/', start);
            if (end != std::string_view::npos)
            {
                segment = path_view.substr(start, end - start);
                path_view.remove_prefix(end);
            }
            else
            {
                segment = path_view.substr(start);
                path_view.remove_prefix(path_view.size());
            }
            current = segment_handler(current, std::string(segment));
        }
    }
    return current;
}

class evhttp_route_mapper
{
public:
    evhttp_route_mapper() = default;
    ~evhttp_route_mapper() = default;
    evhttp_router *operator()(evhttp_router *router, const std::string &segment)
    {
        evhttp_router *next;
        if (segment == "*")
        {
            next = router->get_wildcard(true);
        }
        else
        {
            next = router->get_child(segment, true);
        }
        return next;
    }
};

class evhttp_route_matcher
{
private:
    std::vector<std::string> &_path_vars;

public:
    evhttp_route_matcher(std::vector<std::string> &path_vars) : _path_vars(path_vars)
    {
    }
    ~evhttp_route_matcher() = default;
    evhttp_router *operator()(evhttp_router *router, const std::string &segment)
    {
        evhttp_router *next = router->get_child(segment, false);
        if (next == nullptr)
        {
            next = router->get_wildcard(false);
            if (next != nullptr)
            {
                _path_vars.push_back(segment);
            }
        }
        return next;
    }
};

static void evhttp_router_cb(evhttp_request *req, void *arg)
{

    evhttp_router *router = static_cast<evhttp_router *>(arg);
    std::vector<std::string> path_vars;
    evhttp_route_matcher matcher(path_vars);
    const char *path = evhttp_uri_get_path(evhttp_request_get_evhttp_uri(req));
    evhttp_router *match = evhttp_route(router, path, matcher);
    if (!match)
    {
        match = router;
        path_vars.clear();
    }

    auto [handler, handler_arg] = match->get_handler();
    if (handler != nullptr)
    {
        evhttp_pathvars vars(path_vars);
        void (*cb)(struct evhttp_request *req, const struct evhttp_pathvars *vars, void *arg);
        switch (evhttp_request_get_command(req))
        {
        case EVHTTP_REQ_GET:
            cb = handler->get_cb;
            break;
        case EVHTTP_REQ_POST:
            cb = handler->post_cb;
            break;
        case EVHTTP_REQ_HEAD:
            cb = handler->head_cb;
            break;
        case EVHTTP_REQ_PUT:
            cb = handler->put_cb;
            break;
        case EVHTTP_REQ_DELETE:
            cb = handler->delete_cb;
            break;
        case EVHTTP_REQ_OPTIONS:
            cb = handler->options_cb;
            break;
        case EVHTTP_REQ_TRACE:
            cb = handler->trace_cb;
            break;
        case EVHTTP_REQ_PATCH:
            cb = handler->patch_cb;
            break;
        }
        if (cb != nullptr)
        {
            cb(req, &vars, handler_arg);
        }
        else
        {
            evhttp_send_error(req, HTTP_BADMETHOD, "Method Not Allowed");
        }
    }
    else
    {
        evhttp_send_error(req, HTTP_NOTFOUND, "Not Found");
    }
}

const char *evhttp_pathvars_get(const evhttp_pathvars *vars, int index)
{
    return vars->get(index);
}

int evhttp_pathvars_size(const evhttp_pathvars *vars)
{
    return vars->size();
}

evhttp_router *evhttp_router_new(evhttp *http)
{
    evhttp_router *result = new evhttp_router();
    evhttp_set_gencb(http, evhttp_router_cb, result);
    return result;
}

void evhttp_router_free(evhttp_router *router)
{
    delete router;
}

evhttp_router *evhttp_router_handle(evhttp_router *router, const char *pattern, const evhttp_handler *handler, void *arg)
{
    evhttp_route_mapper mapper;
    const char *path = (pattern != nullptr) ? pattern : "";
    evhttp_router *node = evhttp_route(router, path, mapper);
    if (node != nullptr)
    {
        node->set_handler(handler, arg);
    }
    return node;
}