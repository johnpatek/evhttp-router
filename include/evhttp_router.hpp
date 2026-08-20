/**
 * Copyright (c) 2026 John R Patek Sr
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef EVHTTP_ROUTER_HPP
#define EVHTTP_ROUTER_HPP


/**
 * @file evhttp_router.hpp
 * @brief C++ wrapper for evhttp_router.h
 * @note This file provides a C++ wrapper for the evhttp_router.h header file,
 * allowing for easier integration with C++ projects. It provides classes for handling HTTP 
 * requests and routing them to the appropriate handlers based on URL patterns.
 * @author John R Patek Sr
 */

#include <evhttp_router.h>

#include <memory>
#include <string>

namespace evhttprouter
{
    /**
     * @class PathVars
     * @brief wrapper for evhttp_pathvars struct
     */
    class PathVars
    {
    private:
        const struct evhttp_pathvars *vars_;

    public:
        /**
         * @brief Construct a new PathVars object
         * @param vars pointer to evhttp_pathvars struct
         * @note This constructor does not take ownership of the pointer, as it is not
         * managed by the handler. The pointer will be invalid after the handler returns,
         * so any path variables must be copied if they are needed after the handler is done.
         */
        PathVars(const struct evhttp_pathvars *vars) : vars_(vars) {}
        ~PathVars() = default;
        const char *get(int index) const { return evhttp_pathvars_get(vars_, index); }
        int size() const { return evhttp_pathvars_size(vars_); }
    };

    /**
     * @class Handler
     * @brief base class for handling HTTP requests
     */
    class Handler
    {
    private:
        static void method_not_allowed(struct evhttp_request *req)
        {
            evhttp_send_error(req, 405, "Method Not Allowed");
        }

    public:
        /**
         * @brief Construct a new Handler object
         * @note This constructor allows the base class to be used as a handler. It has little use in
         * most cases, as it will return a bad method error for every request.
         */
        Handler() = default;

        /**
         * @brief Destroy the Handler object
         * @note This destructor is virtual to allow for proper cleanup of derived classes.
         */
        virtual ~Handler() = default;

        /**
         * @brief Handle a GET request
         * @param req pointer to evhttp_request struct
         * @param vars PathVars object containing the path variables for the request
         * @note This method should be overridden by derived classes to handle GET requests. The default
         * implementation will return a 405 Method Not Allowed error.
         */
        virtual void onGet(struct evhttp_request *req, const PathVars &vars) { method_not_allowed(req); }

        /**
         * @brief Handle a POST request
         * @param req pointer to evhttp_request struct
         * @param vars PathVars object containing the path variables for the request
         * @note This method should be overridden by derived classes to handle POST requests. The default
         * implementation will return a 405 Method Not Allowed error.
         */
        virtual void onPost(struct evhttp_request *req, const PathVars &vars) { method_not_allowed(req); }

        /**
         * @brief Handle a HEAD request
         * @param req pointer to evhttp_request struct
         * @param vars PathVars object containing the path variables for the request
         * @note This method should be overridden by derived classes to handle HEAD requests. The default
         * implementation will return a 405 Method Not Allowed error.
         */
        virtual void onHead(struct evhttp_request *req, const PathVars &vars) { method_not_allowed(req); }

        /**
         * @brief Handle a PUT request
         * @param req pointer to evhttp_request struct
         * @param vars PathVars object containing the path variables for the request
         * @note This method should be overridden by derived classes to handle PUT requests. The default
         * implementation will return a 405 Method Not Allowed error.
         */
        virtual void onPut(struct evhttp_request *req, const PathVars &vars) { method_not_allowed(req); }

        /**
         * @brief Handle a DELETE request
         * @param req pointer to evhttp_request struct
         * @param vars PathVars object containing the path variables for the request
         * @note This method should be overridden by derived classes to handle DELETE requests. The default
         * implementation will return a 405 Method Not Allowed error.
         */
        virtual void onDelete(struct evhttp_request *req, const PathVars &vars) { method_not_allowed(req); }

        /**
         * @brief Handle an OPTIONS request
         * @param req pointer to evhttp_request struct
         * @param vars PathVars object containing the path variables for the request
         * @note This method should be overridden by derived classes to handle OPTIONS requests. The default
         * implementation will return a 405 Method Not Allowed error.
         */
        virtual void onOptions(struct evhttp_request *req, const PathVars &vars) { method_not_allowed(req); }

        /**
         * @brief Handle a TRACE request
         * @param req pointer to evhttp_request struct
         * @param vars PathVars object containing the path variables for the request
         * @note This method should be overridden by derived classes to handle TRACE requests. The default
         * implementation will return a 405 Method Not Allowed error.
         */
        virtual void onTrace(struct evhttp_request *req, const PathVars &vars) { method_not_allowed(req); }

        /**
         * @brief Handle a PATCH request
         * @param req pointer to evhttp_request struct
         * @param vars PathVars object containing the path variables for the request
         * @note This method should be overridden by derived classes to handle PATCH requests. The default
         * implementation will return a 405 Method Not Allowed error.
         */
        virtual void onPatch(struct evhttp_request *req, const PathVars &vars) { method_not_allowed(req); }
    };

    /**
     * @class Router
     * @brief wrapper for evhttp_router struct
     */
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
        /**
         * @brief Construct a new Router object
         * @param http pointer to evhttp struct
         */
        explicit Router(evhttp *http) : _router(evhttp_router_new(http), &evhttp_router_free) {}

        /**
         * @brief Construct a new Router object
         * @param router pointer to evhttp_router struct
         * @note This constructor takes ownership of the router pointer and will free it when the Router object is destroyed.
         */
        explicit Router(evhttp_router *router) : _router(router, &evhttp_router_free) {}

        /**
         * @brief Destroy the Router object
         * @note This will free the underlying evhttp_router struct.
         */
        ~Router() = default;

        /**
         * @brief Handle a request with the given pattern and handler
         * @param pattern the URL pattern to match
         * @param handler the handler to handle the request
         * @return true if the request was handled successfully, false otherwise
         * @note The handler must be a subclass of evhttprouter::Handler and must remain valid for the lifetime of the Router
         * object.
         */
        bool handle(const char *pattern, Handler *handler)
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
            return evhttp_router_handle(_router.get(), pattern, &ev_handler, handler) != nullptr;
        }
    };
}

#endif