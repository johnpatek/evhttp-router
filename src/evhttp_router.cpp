#include <evhttp_router.h>

#include <array>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

static void evhttp_router_cb(struct evhttp_request *req, void *arg);

struct evhttp_pathvars
{
    std::vector<std::string> vars;
};

struct evhttp_router
{
    using callback = void (*)(struct evhttp_request *req, const struct evhttp_pathvars *vars, void *arg);
    class functionmap
    {
    private:
        static uint8_t cmd_to_index(evhttp_cmd_type cmd)
        {
            const uint8_t lut[11] = {
                255, // skip
                0,   // 1 % 11   = 1  -> log2(1)   = 0
                1,   // 2 % 11   = 2  -> log2(2)   = 1
                8,   // 256 % 11 = 3  -> log2(256) = 8
                2,   // 4 % 11   = 4  -> log2(4)   = 2
                4,   // 16 % 11  = 5  -> log2(16)  = 4
                255, // skip
                7,   // 128 % 11 = 7  -> log2(128) = 7
                3,   // 8 % 11   = 8  -> log2(8)   = 3
                6,   // 64 % 11  = 9  -> log2(64)  = 6
                5,   // 32 % 11  = 10 -> log2(32)  = 5
            };
            return lut[cmd % 11];
        }
        std::array<callback, 9> _callbacks;
        void *_arg;

    public:
        functionmap(const evhttp_handler *handler, void *arg) : _arg(arg)
        {
            _callbacks[cmd_to_index(EVHTTP_REQ_GET)] = handler->get_cb;
            _callbacks[cmd_to_index(EVHTTP_REQ_POST)] = handler->post_cb;
            _callbacks[cmd_to_index(EVHTTP_REQ_HEAD)] = handler->head_cb;
            _callbacks[cmd_to_index(EVHTTP_REQ_PUT)] = handler->put_cb;
            _callbacks[cmd_to_index(EVHTTP_REQ_DELETE)] = handler->delete_cb;
            _callbacks[cmd_to_index(EVHTTP_REQ_OPTIONS)] = handler->options_cb;
            _callbacks[cmd_to_index(EVHTTP_REQ_TRACE)] = handler->trace_cb;
            _callbacks[cmd_to_index(EVHTTP_REQ_CONNECT)] = handler->connect_cb;
            _callbacks[cmd_to_index(EVHTTP_REQ_PATCH)] = handler->patch_cb;
        }

        callback cmd_cb(evhttp_cmd_type cmd) const
        {
            return _callbacks[cmd_to_index(cmd)];
        }

        void *arg() const
        {
            return _arg;
        }
    };
    std::unordered_map<std::string, std::unique_ptr<evhttp_router>> child_nodes;
    std::unique_ptr<evhttp_router> wildcard_node;
    std::unique_ptr<functionmap> callbacks;
};

const char *evhttp_pathvars_get(const struct evhttp_pathvars *vars, int index)
{
    return vars->vars[index].c_str();
}

int evhttp_pathvars_size(const struct evhttp_pathvars *vars)
{
    return vars->vars.size();
}

evhttp_router *evhttp_router_new(struct evhttp *http)
{
    evhttp_router *result = new evhttp_router();
    evhttp_set_gencb(http, evhttp_router_cb, result);
    return result;
}

void evhttp_router_free(struct evhttp_router *router)
{
    delete router;
}

void evhttp_router_handle(struct evhttp_router *router, const char *pattern, const struct evhttp_handler *handler, void *arg)
{
    if (pattern != nullptr)
    {
        std::string_view pattern_view(pattern);
        size_t position = pattern_view.find_first_not_of('/');
        size_t next_position;
        evhttp_router *current = router;

        while (position != std::string_view::npos)
        {
            next_position = pattern_view.find('/', position);
            std::string_view segment = pattern_view.substr(position, next_position - position);
            std::cerr << segment << " ";
            if (segment == "*")
            {
                if (!current->wildcard_node)
                {
                    current->wildcard_node.reset(new evhttp_router());
                }
                current = current->wildcard_node.get();
            }
            else
            {
                auto it = current->child_nodes.find(std::string(segment));
                if (it == current->child_nodes.end())
                {
                    auto new_node = std::make_unique<evhttp_router>();
                    current->child_nodes[std::string(segment)] = std::move(new_node);
                    current = current->child_nodes[std::string(segment)].get();
                }
                else
                {
                    current = it->second.get();
                }
            }

            position = (next_position == std::string_view::npos) ? std::string_view::npos : next_position + 1;
        }
        std::cerr << std::endl;

        current->callbacks.reset(new evhttp_router::functionmap(handler, arg));
    }
    else
    {
        router->callbacks.reset(new evhttp_router::functionmap(handler, arg));
    }
}

void evhttp_router_cb(struct evhttp_request *req, void *arg)
{
    const char *uri_path = evhttp_uri_get_path(evhttp_request_get_evhttp_uri(req));
    evhttp_router *const root = static_cast<evhttp_router *>(arg);
    evhttp_router *current;
    evhttp_pathvars path_vars;
    evhttp_router::functionmap *callbacks;
   
    size_t position, next_position;
    std::string_view path;

    current = root;
    path = uri_path != nullptr ? std::string_view(uri_path) : std::string_view();
    position = path.find_first_not_of('/');
    
    while (current != nullptr && position != std::string_view::npos)
    {
        next_position = path.find('/', position);
        std::string_view segment = path.substr(position, next_position - position);
        auto it = current->child_nodes.find(std::string(segment));
        if (it != current->child_nodes.end())
        {
            current = it->second.get();
        }
        else if (current->wildcard_node)
        {
            current = current->wildcard_node.get();
            path_vars.vars.emplace_back(segment);
        }
        else
        {
            current = nullptr;
        }
        position = (next_position == std::string_view::npos) ? std::string_view::npos : next_position + 1;
    }

    if (current != nullptr)
    {
        callbacks = current->callbacks.get();
    }
    else
    {
        callbacks = root->callbacks.get();
        path_vars.vars.clear();
    }

    if (callbacks != nullptr)
    {
        evhttp_router::callback cb = callbacks->cmd_cb(evhttp_request_get_command(req));
        if (cb != nullptr)
        {
            
            cb(req, &path_vars, callbacks->arg());
        }
        else
        {
            evhttp_send_reply(req, 405, "Method Not Allowed", nullptr);
        }
    }
    else
    {
        evhttp_send_reply(req, 404, "Not Found", nullptr);
    }
}
