#ifndef EVHTTP_ROUTER_H
#define EVHTTP_ROUTER_H

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

struct evhttp_handler
{
    void (*get_cb)(struct evhttp_request *req, const struct evhttp_pathvars *vars, void *arg);
    void (*post_cb)(struct evhttp_request *req, const struct evhttp_pathvars *vars, void *arg);
    void (*head_cb)(struct evhttp_request *req, const struct evhttp_pathvars *vars, void *arg);
    void (*put_cb)(struct evhttp_request *req, const struct evhttp_pathvars *vars, void *arg);
    void (*delete_cb)(struct evhttp_request *req, const struct evhttp_pathvars *vars, void *arg);
    void (*options_cb)(struct evhttp_request *req, const struct evhttp_pathvars *vars, void *arg);
    void (*trace_cb)(struct evhttp_request *req, const struct evhttp_pathvars *vars, void *arg);
    void (*connect_cb)(struct evhttp_request *req, const struct evhttp_pathvars *vars, void *arg);
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
 * necessary information.
 * @param arg User-defined argument to be passed to the handler callbacks.
 * @return struct evhttp_router* router on success, NULL on failure. A failure will 
 * only occur if the pattern is invalid.
 */
struct evhttp_router *evhttp_router_handle(struct evhttp_router *router, const char *pattern, const struct evhttp_handler *handler, void *arg);

#if defined(__cplusplus)
}
#endif

#endif // EVHTTP_ROUTER_H