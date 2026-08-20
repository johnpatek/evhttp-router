#ifndef EXAMPLE_H
#define EXAMPLE_H

#if defined(__cplusplus)
extern "C" {
#endif

#include <evhttp_router.h>

#include <event2/event.h>
#include <event2/buffer.h>

int example_start(struct evhttp *http);

void example_stop();

#if defined(__cplusplus)
}
#endif

#endif