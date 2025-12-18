#ifndef CONFIG_H
#define CONFIG_H

#include <sys/types.h>

typedef struct {
  u_int16_t port;
} ServerConfig;

int parse_config(ServerConfig *server_config, char *data, size_t data_length);
int load_config(ServerConfig *config, const char *filename);

#endif // !#ifndef CONFIG_H
