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

#ifndef EVHTTP_ROUTER_H
#define EVHTTP_ROUTER_H

/**
 * @file evhttp_router.h
 * @brief C interface for evhttp_router
 * @author John R Patek Sr
 */

#if defined(__cplusplus)
extern "C" {
#endif

#include <event2/http.h>

/**
 * @brief Structure representing path variables extracted from a URL.
 * 
 * For routing patterns that include wildcards, this structure will be 
 * populated with the values at each wildcard position in the order they 
 * appear.
 */
struct evhttp_pathvars;

/**
 * @brief Structure representing an HTTP router that maps URL patterns to
 * corresponding handler functions for different HTTP methods.
 */
struct evhttp_router;

/**
 * @brief Structure representing callbacks for each HTTP method of a given 
 * route. This structure can be initialized such that some, none, or all of 
 * the methods are supported. If a given method is left NULL, it will send 
 * a 405 error. 
 */
struct evhttp_handler
{
    void (*get_cb)(struct evhttp_request *req, const struct evhttp_pathvars *vars, void *arg);
    void (*post_cb)(struct evhttp_request *req, const struct evhttp_pathvars *vars, void *arg);
    void (*head_cb)(struct evhttp_request *req, const struct evhttp_pathvars *vars, void *arg);
    void (*put_cb)(struct evhttp_request *req, const struct evhttp_pathvars *vars, void *arg);
    void (*delete_cb)(struct evhttp_request *req, const struct evhttp_pathvars *vars, void *arg);
    void (*options_cb)(struct evhttp_request *req, const struct evhttp_pathvars *vars, void *arg);
    void (*trace_cb)(struct evhttp_request *req, const struct evhttp_pathvars *vars, void *arg);
    void (*patch_cb)(struct evhttp_request *req, const struct evhttp_pathvars *vars, void *arg);
};

/**
 * @brief Get the path variable at the specified index.
 *
 * @param vars The path variables structure.
 * @param index The index of the path variable to retrieve.
 * @return const char* The path variable at the specified index.
 */
const char *evhttp_pathvars_get(const struct evhttp_pathvars *vars, int index);

/**
 * @brief Get the number of path variables.
 * 
 * @param vars The path variables structure.
 * @return int The number of path variables.
 */
int evhttp_pathvars_size(const struct evhttp_pathvars *vars);

/**
 * @brief Create a new HTTP router.
 * 
 * @param http The HTTP server instance.
 * @return struct evhttp_router* A pointer to the newly created router.
 */
struct evhttp_router *evhttp_router_new(struct evhttp *http);

/**
 * @brief Free the resources associated with the HTTP router.
 * 
 * @param router The router to free.
 */
void evhttp_router_free(struct evhttp_router *router);

/**
 * @brief Handle an HTTP request by routing it to the appropriate handler based on 
 * the specified pattern.
 * 
 * @param router The HTTP router.
 * @param pattern The URL pattern to match against incoming requests. If NULL, the 
 * handler will be used as a default handler for any unmatched requests.
 * @param handler The handler containing callbacks for different HTTP methods. This 
 * struct does not need to remain valid after this call, as the router will copy the 
 * necessary information. If NULL, the existing handler will be removed if it exists.
 * @param arg User-defined argument to be passed to the handler callbacks.
 * @return struct evhttp_router* router on success, NULL on failure. A failure will 
 * only occur if the pattern is invalid.
 */
struct evhttp_router *evhttp_router_handle(struct evhttp_router *router, const char *pattern, const struct evhttp_handler *handler, void *arg);

#if defined(__cplusplus)
}
#endif

#endif // EVHTTP_ROUTER_H