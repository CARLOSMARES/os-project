#ifndef LOG_H
#define LOG_H

#include <stdarg.h>
#include <stdint.h>

void serial_init(void); // COM1 = 0x3F8
void serial_write_char(char c);
void serial_write(const char *s);
int kprintf(const char *fmt, ...);

#define KLOG_INFO(fmt, ...) kprintf("[INFO] " fmt "\n", ##__VA_ARGS__)
#define KLOG_WARN(fmt, ...) kprintf("[WARN] " fmt "\n", ##__VA_ARGS__)
#define KLOG_ERR(fmt, ...) kprintf("[ERR ] " fmt "\n", ##__VA_ARGS__)

#endif
