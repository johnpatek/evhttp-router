#include <evhttp_router.h>

#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

class evhttp_pathvars
{
private:
    std::vector<std::string> vars;

public:
    evhttp_pathvars();
    ~evhttp_pathvars() = default;
};

class evhttp_router
{
private:
    std::unordered_map<std::string, std::unique_ptr<evhttp_router>> _child_nodes;
    std::unique_ptr<evhttp_router> _wildcard_node;
    std::unique_ptr<evhttp_handler> _handler;
    void *_arg;

public:
    evhttp_router();
    ~evhttp_router() = default;
    evhttp_router *get_child(const std::string &segment, bool create_if_missing);
    evhttp_router *get_wildcard(bool create_if_missing);
    std::pair<evhttp_handler *, void *> get_handler();
    void set_handler(const evhttp_handler *handler, void *arg);
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