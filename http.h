#ifndef SERVER_HTTP_H
#define SERVER_HTTP_H
#include <stdlib.h>
#include "server.h"

typedef con_item* connection;

int http_response(connection con);

#endif //SERVER_HTTP_H
