#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define COM1 0x3F8
static inline void outb(uint16_t port, uint8_t val) { __asm__ volatile("outb %0,%1" ::"a"(val), "Nd"(port)); }
static inline uint8_t inb(uint16_t port)
{
    uint8_t ret;
    __asm__ volatile("inb %1,%0" : "=a"(ret) : "Nd"(port));
    return ret;
}
static int is_transmit_empty() { return inb(COM1 + 5) & 0x20; }

void serial_init(void)
{
    outb(COM1 + 1, 0x00); // disable interrupts
    outb(COM1 + 3, 0x80); // enable DLAB
    outb(COM1 + 0, 0x03); // divisor low (38400 baud)
    outb(COM1 + 1, 0x00); // divisor high
    outb(COM1 + 3, 0x03); // 8 bits, no parity, one stop
    outb(COM1 + 2, 0xC7); // enable FIFO
    outb(COM1 + 4, 0x0B); // IRQs enabled, RTS/DSR set
}

void serial_write_char(char c)
{
    if (c == '\n')
    {
        serial_write_char('\r');
    }
    int timeout = 100000;
    while (!is_transmit_empty() && --timeout > 0)
        ;
    outb(COM1, (uint8_t)c);
}

void serial_write(const char *s)
{
    while (*s)
        serial_write_char(*s++);
}

// mínimo printf (soporta %s %d %x %c)
static void itoa(unsigned int val, unsigned int base, char *buf)
{
    static const char *digits = "0123456789abcdef";
    char tmp[32];
    int i = 0;
    if (val == 0)
    {
        buf[0] = '0';
        buf[1] = 0;
        return;
    }
    while (val)
    {
        tmp[i++] = digits[val % base];
        val /= base;
    }
    for (int j = 0; i > 0;)
    {
        buf[j++] = tmp[--i];
        buf[j] = 0;
    }
}

int kprintf(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    int count = 0;
    for (; *fmt; ++fmt)
    {
        if (*fmt != '%')
        {
            serial_write_char(*fmt);
            count++;
            continue;
        }
        ++fmt;
        if (*fmt == 's')
        {
            const char *s = va_arg(ap, const char *);
            serial_write(s);
            while (*s++)
                count++;
        }
        else if (*fmt == 'd')
        {
            char b[32];
            int v = va_arg(ap, int);
            unsigned int u = (v < 0) ? -v : v;
            if (v < 0)
            {
                serial_write_char('-');
                count++;
            }
            itoa(u, 10, b);
            serial_write(b);
            for (char *p = b; *p++;)
                count++;
        }
        else if (*fmt == 'x')
        {
            char b[32];
            itoa(va_arg(ap, unsigned int), 16, b);
            serial_write("0x");
            count += 2;
            serial_write(b);
            for (char *p = b; *p++;)
                count++;
        }
        else if (*fmt == 'c')
        {
            char c = (char)va_arg(ap, int);
            serial_write_char(c);
            count++;
        }
        else
        {
            serial_write_char('%');
            serial_write_char(*fmt);
            count += 2;
        }
    }
    va_end(ap);
    return count;
}
