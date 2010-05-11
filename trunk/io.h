void print_dump(unsigned char* p, unsigned int len);
void print_lenRX(unsigned char* p, unsigned int len);
void print_i (unsigned char i);
unsigned int putchar(unsigned char send_char);
void uart_init(void);
void _print_fstr (const char __flash * s);
void _print_num ( const char __flash * s, UINT16 Num);
void _print_rn (void);

//#define CONSOLE_DEBUG 1