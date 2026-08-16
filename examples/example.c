#include <evhttp_router.h>

#include <event2/event.h>
#include <event2/buffer.h>

#include <signal.h>
#include <string.h>

/**
 * This is a simple GET request to the endpoint /strgen/*, where the wildcared can be either 
 * uuid or ulid. This is not a typical use case, but it demonstrates how to use wildcards with 
 * the router.
 */
static void handle_strgen(struct evhttp_request *req, const struct evhttp_pathvars *vars, void *arg);

/**
 * This is a simple POST request to the endpoint /strlen, which returns a message with the character 
 * count of the posted text.
 */
static void handle_strlen(struct evhttp_request *req, const struct evhttp_pathvars *vars, void *arg);

// Graceful shutdown on SIGINT or SIGTERM
static void handle_signal(int signum);

// Global so the signal handler can access it
static struct event_base *base;

int main(int argc, const char **argv)
{
    const struct evhttp_handler strgen_handler = {
        .get_cb = handle_strgen
    };
    const struct evhttp_handler strlen_handler = {
        .post_cb = handle_strlen
    };
    
    struct evhttp *http;
    struct evhttp_router *router;

    base = event_base_new();
    http = evhttp_new(base);

    router = evhttp_router_new(http);
    evhttp_router_handle(router, "/strgen/*", &strgen_handler, NULL);
    evhttp_router_handle(router, "/strlen", &strlen_handler, NULL);

    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);

    if (evhttp_bind_socket(http, "0.0.0.0", 8080) == 0)
    {
        event_base_dispatch(base);
    }

    evhttp_router_free(router);
    evhttp_free(http);
    event_base_free(base);

    return 0;
}

void handle_strgen(struct evhttp_request *req, const struct evhttp_pathvars *vars, void *arg)
{
    const char *type = evhttp_pathvars_get(vars, 0);
    int code = HTTP_OK;
    const char *reason = "OK";
    struct evbuffer *buffer = evbuffer_new();

    if (strcmp(type, "uuid") == 0)
    {
        // Pretend this is a randomly generated UUID
        evbuffer_add_printf(buffer, "UUID: %s", "550e8400-e29b-41d4-a716-446655440000");
    }
    else if (strcmp(type, "ulid") == 0)
    {
        // Pretend this is a randomly generated ULID
        evbuffer_add_printf(buffer, "ULID: %s", "01ARZ3NDEKTSV4RRFFQ69G5FAV");
    }
    else
    {
        // We can only "find" /strgen/uuid or /strgen/ulid
        code = HTTP_NOTFOUND;
        reason = "Not Found";
        evbuffer_free(buffer);
        buffer = NULL;
    }
    
    evhttp_send_reply(req, code, reason, buffer);
    if(buffer)
    {
        evbuffer_free(buffer);
    }
}

void handle_strlen(struct evhttp_request *req, const struct evhttp_pathvars *vars, void *arg)
{
    struct evbuffer *input_buffer = evhttp_request_get_input_buffer(req);
    struct evbuffer *output_buffer = evbuffer_new();
    
    size_t input_length = evbuffer_get_length(input_buffer);
    evbuffer_add_printf(output_buffer, "Length: %zu", input_length);

    evhttp_send_reply(req, HTTP_OK, "OK", output_buffer);
    evbuffer_free(output_buffer);
}

void handle_signal(int signum)
{
    event_base_loopbreak(base);
}
