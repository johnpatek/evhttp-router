# Examples

Collection of C and C++ examples demonstrating `evhttp-router` usage.

## String Service

A very basic C example with the following endpoints:

`GET /strgen/*` - `*` can be `uuid` or `ulid` to "generate" a unique 
identifier string in one of the two formats.

`POST /strlen` - get the length of a string posted to the server.

## Log Service

C++ example with the following endpoints:

`POST /logs` - post a new log entry, receive an ID.

`GET /logs` - list the IDs for all log messages.

`GET /log/*` - `*` is replaced by the message ID, the log message will 
be returned.

`DELETE /log/*` - `*` is replaced by the message ID, the log message 
will be deleted. This can be verified by a `GET` call.