#include "http.h"
#include <stdio.h>

int main() {
  http_response response;
  init_http_response(&response);

  add_http_header(&response, "Content-Type", "text/html");
  add_http_header(&response, "Connection", "close");

  // printf("Initialezed HTTP response:\n");
  // printf("Status Code: %d\n", response.status_code);
  // printf("Reason: %s\n", response.reason_phrase);
  //
  // for (size_t i = 0; i < response.header_count; i++) {
  //   printf("%s: %s\n", response.headers[i].key, response.headers[i].value);
  // }

  size_t response_length = 0;
  char *response_str = construct_http_response(&response, &response_length);
  printf("Constructed HTTP response (%zu bytes):\n", response_length);
  fprintf(stdout, "%.*s", (int)response_length, response_str);

  free_http_response(&response);
}
