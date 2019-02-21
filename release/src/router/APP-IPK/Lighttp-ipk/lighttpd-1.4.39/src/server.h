#ifndef _SERVER_H_
#define _SERVER_H_

#include "base.h"

int config_read(server *srv, const char *fn);
int config_set_defaults(server *srv);

//magic add for auth
char http_pw_check[512];
char http_pw_check_temp[512];
//magic add for auth

#endif
