#include <evhttp_router.h>

#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include <cstring>

class evhttp_pathvars
{
private:
    const std::vector<std::string>& _vars;
public:
    evhttp_pathvars(const std::vector<std::string>& vars) : _vars(vars) {}
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
    std::unordered_map<std::string, std::unique_ptr<evhttp_router>> _child_nodes;
    std::unique_ptr<evhttp_router> _wildcard_node;
    std::unique_ptr<evhttp_handler> _handler;
    void *_arg;

public:
    evhttp_router() = default;
    ~evhttp_router() = default;
    evhttp_router *get_child(const std::string &segment, bool create_if_missing)
    {
        evhttp_router *result(nullptr);
        auto it = _child_nodes.find(segment);
        if (it == _child_nodes.end())
        {
            if (create_if_missing)
            {
                auto new_node = std::make_unique<evhttp_router>();
                result = new_node.get();
                _child_nodes[segment] = std::move(new_node);
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

    }
    
    std::pair<evhttp_handler *, void *> get_handler()
    {
        return { _handler.get(), _arg };
    }

    void set_handler(const evhttp_handler *handler, void *arg)
    {
        evhttp_handler *new_handler(nullptr);
        if (handler != nullptr)
        {
            new_handler = new(evhttp_handler);
            std::memcpy(new_handler, handler, sizeof(evhttp_handler));
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
    const char *segment_start;
    const char *segment_end;

    if (!path_view.empty())
    {
        segment_start = path_view.begin();
        if (path_view[0] == '/')
        {
            segment_start++;
        }

        while (current != nullptr && segment_start != path_view.end())
        {
            segment_end = std::find(segment_start, path_view.end(), '/');
            std::string segment(segment_start, segment_end);
            current = segment_handler(current, segment);
            segment_start = segment_end;
        }
    }

    return current;
}

class evhttp_route_mapper
{
public:
    evhttp_route_mapper() = default;
    ~evhttp_route_mapper() = default;
    evhttp_router *operator()(evhttp_router *router, const std::string& segment)
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
    std::vector<std::string> _path_vars;
public:
    evhttp_route_matcher() = default;
    ~evhttp_route_matcher() = default;
    evhttp_router *operator()(evhttp_router *router, const std::string& segment)
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