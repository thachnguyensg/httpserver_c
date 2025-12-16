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

void init_http_response(http_response *response) {
  response->status_code = 200;
  strncpy(response->reason_phrase, "OK", HTTP_MAX_REASON_LEN - 1);
  response->headers = NULL;
  response->header_count = 0;
  response->body = NULL;
  response->body_length = 0;
}

void add_http_header(http_response *response, const char *key,
                     const char *value) {
  response->headers = realloc(
      response->headers, sizeof(http_header_t) * (response->header_count + 1));
  if (!response->headers) {
    perror("realloc");
    exit(EXIT_FAILURE);
  }

  strncpy(response->headers[response->header_count].key, key,
          HTTP_MAX_HEADER_KEY_LEN - 1);
  strncpy(response->headers[response->header_count].value, value,
          HTTP_MAX_HEADER_VAL_LEN - 1);

  response->header_count++;
}

void set_http_body(http_response *response, const char *body) {
  if (response->body) {
    free(response->body);
  }

  response->body_length = strlen(body);
  response->body = malloc(response->body_length);
  if (!response->body) {
    perror("malloc");
    exit(EXIT_FAILURE);
  }
  memcpy(response->body, body, response->body_length);
}

void free_http_response(http_response *response) {
  if (response->headers) {
    free(response->headers);
    response->headers = NULL;
  }
  response->header_count = 0;

  if (response->body) {
    free(response->body);
    response->body = NULL;
  }
  response->body_length = 0;
}

char *construct_http_response(const http_response *response,
                              size_t *response_length) {
  size_t buffer_size = 1024; // Initial size
  char *buffer = malloc(buffer_size);

  size_t offset = 0;
  offset = snprintf(buffer, buffer_size, "HTTP/1.1 %d %s\r\n",
                    response->status_code, response->reason_phrase);

  for (size_t i = 0; i < response->header_count; i++) {
    // trick to check required buffer size by passing NULL buffer and 0 length
    // to snprintf
    size_t header_length =
        snprintf(NULL, 0, "%s: %s\r\n", response->headers[i].key,
                 response->headers[i].value);
    while (buffer_size - offset - header_length - 1 - 2 <= 0) {
      buffer_size *= 2;
      buffer = realloc(buffer, buffer_size);
    }

    offset += snprintf(buffer + offset, buffer_size - offset, "%s: %s\r\n",
                       response->headers[i].key, response->headers[i].value);
  }
  offset += snprintf(buffer + offset, buffer_size - offset, "\r\n");

  if (response->body) {
    while (buffer_size - offset - 1 < response->body_length) {
      buffer_size = offset + response->body_length;
      buffer = realloc(buffer, buffer_size);
    }

    memcpy(buffer + offset, response->body, response->body_length);
    offset += response->body_length;
  }

  *response_length = offset;
  return buffer;
}

void send_http_response(int socket_fd, const http_response *response) {
  size_t response_length;
  char *response_buffer = construct_http_response(response, &response_length);

  size_t total_sent = 0;
  while (total_sent < response_length) {
    ssize_t sent = send(socket_fd, response_buffer + total_sent,
                        response_length - total_sent, 0);
    if (sent <= 0) {
      perror("send");
      break;
    }
    total_sent += sent;
  }

  free(response_buffer);
}

void sanitize_path(const char *requested_path, char *sanitized_path,
                   size_t buffer_size) {
  const char *web_root = "./www";
  snprintf(sanitized_path, buffer_size, "%s%s", web_root, requested_path);

  if (strstr(requested_path, "..") != NULL) {
    snprintf(sanitized_path, buffer_size, "%s%s", web_root, "/404.html");
  }
}

void serve_file(const char *path, http_response *response) {
  FILE *file = fopen(path, "rb");
  if (!file) {
    response->status_code = 404;
    strncpy(response->reason_phrase, "Not Found", HTTP_MAX_REASON_LEN - 1);
    file = fopen("./www/404.html", "rb");
  }

  // Determine file size
  fseek(file, 0, SEEK_END);
  size_t file_size = ftell(file);
  fseek(file, 0, SEEK_SET);

  char *file_content = malloc(file_size);
  if (!file_content) {
    perror("Failed to allocate memory for file content");
    fclose(file);
    exit(EXIT_FAILURE);
  }

  fread(file_content, 1, file_size, file);
  fclose(file);

  response->body = file_content;
  response->body_length = file_size;

  // determine content type based on file extension
  if (strstr(path, ".html")) {
    add_http_header(response, "Content-Type", "text/html");
  } else if (strstr(path, ".css")) {
    add_http_header(response, "Content-Type", "text/css");
  } else if (strstr(path, ".js")) {
    add_http_header(response, "Content-Type", "application/javascript");
  } else if (strstr(path, ".png")) {
    add_http_header(response, "Content-Type", "image/png");
  } else if (strstr(path, ".jpg") || strstr(path, ".jpeg")) {
    add_http_header(response, "Content-Type", "image/jpeg");
  } else {
    add_http_header(response, "Content-Type", "application/octet-stream");
  }

  char content_length_str[32];
  snprintf(content_length_str, sizeof(content_length_str), "%zu",
           response->body_length);
  add_http_header(response, "Content-Length", content_length_str);

  free(file_content);
}
