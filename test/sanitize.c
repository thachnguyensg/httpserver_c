#include "http.h"
#include <stdio.h>
int main() {
  char path[1024];
  sanitize_path("/etc/passwd", path, sizeof(path));
  printf("%s\n", path);
}
