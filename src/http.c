#include "http.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

int read_http_request(int socket_fd, http_request *request) {
  char buf[8192] = {0};
  ssize_t bytes = recv(socket_fd, buf, sizeof(buf) - 1, 0);
  if (bytes <= 0) {
    perror("recv");
    return -1;
  }

  buf[bytes] = '\0';

  if (sscanf(buf, "%7s %2047s %15s", request->method, request->path,
             request->protocol) != 3) {
    fprintf(stderr, "Failed to parse HTTP request\n");
    return -1;
  }

  return 0;
}

int parse_http_headers(const char *raw_request, http_request *request) {
  char *line_start = strstr(raw_request, "\r\n");
  if (!line_start) {
    return HTTP_PARSE_INVALID;
  }

  line_start += 2; // Move past the request line
  while (line_start && *line_start != '\0' && *line_start != '\r' &&
         *line_start != '\n') {
    char *line_end = strstr(line_start, "\r\n");
    if (!line_end || line_start == line_end) {
      break; // End of headers
    }

    char key[256] = {0};
    char value[512] = {0};
    if (sscanf(line_start, "%255[^:]: %511[^\r\n]", key, value) == 2) {
      request->headers =
          realloc(request->headers,
                  sizeof(http_header_t) * (request->header_count + 1));
      if (!request->headers) {
        return HTTP_PARSE_INVALID;
      }

      strncpy(request->headers[request->header_count].key, key,
              sizeof(request->headers[request->header_count].key) - 1);
      strncpy(request->headers[request->header_count].value, value,
              sizeof(request->headers[request->header_count].value) - 1);

      request->header_count++;
    }

    line_start = line_end + 2; // Move to the next line
  }
  return HTTP_PARSE_OK;
}

void free_http_request(http_request *request) {
  if (request->headers) {
    free(request->headers);
    request->headers = NULL;
  }
  request->header_count = 0;
}
