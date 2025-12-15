#include "http.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

int read_http_request(int socket_fd, http_request *request) {
  ssize_t bytes =
      recv(socket_fd, request->buffer, sizeof(request->buffer) - 1, 0);
  if (bytes <= 0) {
    perror("recv");
    return -1;
  }

  request->buffer[bytes] = '\0';

  return 0;
}

http_method_e http_method_to_enum(const char *method) {
  if (strcmp(method, "GET") == 0) {
    return HTTP_METHOD_GET;
  } else if (strcmp(method, "POST") == 0) {
    return HTTP_METHOD_POST;
  } else if (strcmp(method, "PUT") == 0) {
    return HTTP_METHOD_PUT;
  } else {
    return HTTP_METHOD_UNK;
  }
}

http_parse_e parse_request_line(const char *raw_request,
                                http_request *request) {
  if (sscanf(raw_request, "%7s %2047s %15s", request->method, request->path,
             request->protocol) != 3) {
    return HTTP_PARSE_INVALID;
  }

  request->methode = http_method_to_enum(request->method);

  if (strcmp(request->protocol, "HTTP/1.1") != 0 &&
      strcmp(request->protocol, "HTTP/1.0") != 0) {
    return HTTP_PARSE_INVALID;
  }

  return HTTP_PARSE_OK;
}

http_parse_e parse_http_headers(const char *raw_request,
                                http_request *request) {
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

    char line[HTTP_MAX_HEADER_KEY_LEN + HTTP_MAX_HEADER_VAL_LEN + 1] = {0};
    strncpy(line, line_start, line_end - line_start);

    char *colon_pos = strchr(line, ':');
    if (!colon_pos) {
      break;
    }

    *colon_pos = '\0';

    char *key = line;
    char *value = colon_pos + 1;

    while (*value == ' ') {
      value++; // Skip leading spaces
    }

    request->headers = realloc(
        request->headers, sizeof(http_header_t) * (request->header_count + 1));
    if (!request->headers) {
      return HTTP_PARSE_INVALID;
    }

    strncpy(request->headers[request->header_count].key, key,
            HTTP_MAX_HEADER_KEY_LEN - 1);
    strncpy(request->headers[request->header_count].value, value,
            HTTP_MAX_HEADER_VAL_LEN - 1);

    request->header_count++;

    line_start = line_end + 2; // Move to the next line
  }
  return HTTP_PARSE_OK;
}

void free_http_headers(http_request *request) {
  if (request->headers) {
    free(request->headers);
    request->headers = NULL;
  }
  request->header_count = 0;
}

http_parse_e parse_http_request(const char *raw_request,
                                http_request *request) {
  if (sscanf(raw_request, "%7s %2047s %15s", request->method, request->path,
             request->protocol) != 3) {
    return HTTP_PARSE_INVALID;
  }

  return parse_http_headers(raw_request, request);
}
