#pragma once
#include <stdint.h>
void init_connection(const char* ip, int port);
void send_ping_request();
void send_logreset_request();
void send_logdump_request();
uint64_t send_update_request(int key);
