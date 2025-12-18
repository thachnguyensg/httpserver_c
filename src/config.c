
#include "config.h"
#include "cJSON.h"
#include <stddef.h>
#include <stdio.h>

int load_config(ServerConfig *config, const char *filename) {
  FILE *file = fopen(filename, "r");
  if (!file) {
    perror("Failed to open config file");
    return -1; // Failed to open file
  }

  fseek(file, 0, SEEK_END);
  size_t content_length = ftell(file);
  fseek(file, 0, SEEK_SET);

  char buffer[content_length];
  fread(buffer, 1, content_length, file);
  fclose(file);
  return parse_config(config, buffer, content_length);
}

int parse_config(ServerConfig *server_config, char *data, size_t data_length) {
  int status = 0;
  const cJSON *server = NULL;

  cJSON *config_json = cJSON_ParseWithLength(data, data_length);
  if (!config_json) {
    const char *error_ptr = cJSON_GetErrorPtr();
    if (error_ptr != NULL) {
      fprintf(stderr, "Error before: %s\n", error_ptr);
    }
    status = -1;
    goto end;
  }

  server = cJSON_GetObjectItemCaseSensitive(config_json, "server");
  if (server && cJSON_IsObject(server)) {
    cJSON *port = cJSON_GetObjectItemCaseSensitive(server, "port");
    if (!port || !cJSON_IsNumber(port)) {
      fprintf(stderr, "Missing or invalid 'port' in config\n");
      status = -1;
      goto end;
    }
    if (port->valueint <= 1024 || port->valueint > 65535) {
      fprintf(stderr, "'port' must be between 1024 and 65535\n");
      status = -1;
      goto end;
    }

    server_config->port = port->valueint;
  }

end:
  cJSON_Delete(config_json);
  return status;
}
