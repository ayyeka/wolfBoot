
#include "config.h"
#include "leuart.h"
#include "uart.h"
#define _IO_RECV(buf, len) uart_recv(0, buf, len, (u32)(-1))
#define _IO_SEND(buf, len) uart_send(0, buf, len)