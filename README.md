# Libevent HTTP Router

C/C++ extension for HTTP handling with libevent

## About

`evhttp-router` is a simple extension of the `evhttp` interface, with 
improved support for pattern based routing and per-method HTTP handler 
functions. It is intended to replace the existing `evhttp_set_cb` and 
`evhttp_set_gencb` functions.

## Usage

`evhttp-router` has both a C and C++ interface. It is a static library 
with no additional dependencies besides libevent.

### Integration



### Limitations

To avoid making this extension bloated, overcomplicated, or intrusive, 
most design decisions were made with a bias towards simplicity. As a 
result, the following limitation should be noted:

+ thread safety - the underlying routing table is not protected by any 
synchronization primitives such that routes should be added while there 
are requests being served. While it is possible to add handlers at any 
time, it must be done on a single thread. It is therefore recommended 
that all routes are defined before the `event_base` is dispatched. Once 
the loop is running, requests can be served from multiple threads, as 
the table is only modified when new handlers are added.

+ pattern validation - there are no safeguards against defining routes 
that are ambiguous. If two routes map to the same location in the table, 
the more recent handler will overwrite the existing handler. While there 
is a basic mechanism for rejecting invalid patterns, any valid pattern 
will be mapped without further validation.

+ runtime safeguards - in an effort to minimize both source and binary 
sizes, there are no checks or assertions for errors like array boundary 
violations or unexpected `NULL` handles. This extension is intended to 
augment code that already works, so incorret usage will lead to crashes 
and segmentation faults. To mitigate potential problems in this approach, 
the API is thorougly documented and tested.