
__attribute__((used)) void main(void)
{
    volatile char *vga = (volatile char *)0xB8000;
    vga[40] = 'O'; // columna 20, fila 0
    vga[41] = 0x0F;
    while (1)
    {
        __asm__ volatile("hlt");
    }
}