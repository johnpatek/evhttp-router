#ifndef EVHTTP_ROUTER_HPP
#define EVHTTP_ROUTER_HPP

#include <evhttp_router.h>

#include <memory>
#include <string>

namespace evhttprouter
{
    class PathVars
    {
    private:
        const struct evhttp_pathvars *vars_;

    public:
        PathVars(const struct evhttp_pathvars *vars) : vars_(vars) {}
        ~PathVars() = default;
        const char *get(int index) const { return evhttp_pathvars_get(vars_, index); }
        int size() const { return evhttp_pathvars_size(vars_); }
    };

    class Handler
    {
    private:
        static void method_not_allowed(struct evhttp_request *req)
        {
            evhttp_send_error(req, 405, "Method Not Allowed");
        }

    public:
        Handler() = default;
        virtual ~Handler() = default;
        virtual void onGet(struct evhttp_request *req, const PathVars &vars) { method_not_allowed(req); }
        virtual void onPost(struct evhttp_request *req, const PathVars &vars) { method_not_allowed(req); }
        virtual void onHead(struct evhttp_request *req, const PathVars &vars) { method_not_allowed(req); }
        virtual void onPut(struct evhttp_request *req, const PathVars &vars) { method_not_allowed(req); }
        virtual void onDelete(struct evhttp_request *req, const PathVars &vars) { method_not_allowed(req); }
        virtual void onOptions(struct evhttp_request *req, const PathVars &vars) { method_not_allowed(req); }
        virtual void onTrace(struct evhttp_request *req, const PathVars &vars) { method_not_allowed(req); }
        virtual void onPatch(struct evhttp_request *req, const PathVars &vars) { method_not_allowed(req); }
    };

    class Router
    {
    private:
        std::unique_ptr<evhttp_router, decltype(&evhttp_router_free)> _router;
        template <void (Handler::*method)(struct evhttp_request *, const PathVars &)>
        static void callback(struct evhttp_request *req, const struct evhttp_pathvars *vars, void *arg)
        {
            Handler *handler = static_cast<Handler *>(arg);
            PathVars path_vars(vars);
            (handler->*method)(req, path_vars);
        }

    public:
        Router(evhttp *http) : _router(evhttp_router_new(http), &evhttp_router_free) {}
        ~Router() = default;
        void handle(const char *pattern, Handler *handler)
        {
            const static struct evhttp_handler ev_handler{
                .get_cb = callback<&Handler::onGet>,
                .post_cb = callback<&Handler::onPost>,
                .head_cb = callback<&Handler::onHead>,
                .put_cb = callback<&Handler::onPut>,
                .delete_cb = callback<&Handler::onDelete>,
                .options_cb = callback<&Handler::onOptions>,
                .trace_cb = callback<&Handler::onTrace>,
                .patch_cb = callback<&Handler::onPatch>,
            };
            evhttp_router_handle(_router.get(), pattern, &ev_handler, handler);
        }
    };
}

#endif