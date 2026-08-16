#include <evhttp_router.ipp>

static void evhttp_router_cb(evhttp_request  *req, void *arg)
{
    evhttp_router *router = static_cast<evhttp_router *>(arg);
    evhttp_route_matcher matcher;
    const char *path = evhttp_uri_get_path(evhttp_request_get_evhttp_uri(req));
    evhttp_router *match = evhttp_route(router, path, matcher);
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

struct evhttp_router *evhttp_router_handle(struct evhttp_router *router, const char *pattern, const struct evhttp_handler *handler, void *arg)
{
    evhttp_route_mapper mapper;
    evhttp_router *node = evhttp_route(router, pattern, mapper);
    if (node != nullptr)
    {
        node->set_handler(handler, arg);
    }
    return node;
}