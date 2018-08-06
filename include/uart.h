#ifndef _uart_h
#define _uart_h

void uart_init( void );
void uart_start( void );
void uart_stop( void );

void uart_etxint( void );
void uart_dtxint( void );
void uart_erxint( void );
void uart_drxint( void );

void uart_putchar( const char c );
char uart_getchar( void );

unsigned int uart_sync_write( const char* buff, const unsigned int len );
unsigned int uart_sync_write_to( const char* buff, const unsigned int len, const char endchar );
void uart_async_write( char* buff, const unsigned int len );

unsigned int uart_sync_read( char* buff, const unsigned int len );
unsigned int uart_sync_read_to( char* buff, const unsigned int len, const char endchar );
void uart_async_read( char* buff, const unsigned int len );

#endif /* _uart_h */
