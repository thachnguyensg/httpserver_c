#ifndef HTTP_H
#define HTTP_H

#define HTTP_METHOD_MAX_LEN 8    // Based on maximum method length in HTTP/1.1
#define HTTP_PATH_MAX_LEN 2048   // Practical limit for URIs
#define HTTP_PROTOCOL_MAX_LEN 16 // Standard protocol length (e.g., HTTP/1.1)

typedef enum {
  HTTP_PARSE_OK,
  HTTP_PARSE_INVALID,
} http_parse_e;

typedef struct {
  char method[HTTP_METHOD_MAX_LEN];
  char path[HTTP_PATH_MAX_LEN];
  char protocol[HTTP_PROTOCOL_MAX_LEN];
} http_request;

int read_http_request(int socket_fd, http_request *request);

#endif // HTTP_H
