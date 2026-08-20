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

#include <example.h>
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

static struct evhttp_router *router;

int example_start(struct evhttp *http)
{
    const struct evhttp_handler strgen_handler = {
        .get_cb = handle_strgen};
    const struct evhttp_handler strlen_handler = {
        .post_cb = handle_strlen};
    int rc;
    router = evhttp_router_new(http);
    rc = (evhttp_router_handle(router, "/strgen/*", &strgen_handler, NULL) != NULL) ? 0 : 1;
    if (rc == 0)
    {
        rc = (evhttp_router_handle(router, "/strlen", &strlen_handler, NULL) != NULL) ? 0 : 1;
    }
    return rc;
}

void example_stop()
{
    evhttp_router_free(router);
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
    if (buffer)
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