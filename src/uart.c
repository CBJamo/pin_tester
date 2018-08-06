#define BAUD 115200

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/setbaud.h>

#include "uart.h"

void uart_init( void )
{
  // Define Baud rate and 2x speed with the help of the setbaud macro
  UBRR0H = UBRRH_VALUE;
  UBRR0L = UBRRL_VALUE;
  #if USE_2X
  UCSR0A |= (1 << U2X0);
  #else
  UCSR0A &= ~(1 << U2X0);
  #endif

  // 8-bit data
  UCSR0C |= (1 << USBS0) | (1 << UCSZ00) | (1 << UCSZ00);
}

void uart_start( void )
{
  // Enable TX & RX circutry
  UCSR0B |= (1 << RXEN0) | (1 << TXEN0);
}

void uart_stop( void )
{
  // Disable TX & RX circutry, and TX & RX interupts
  UCSR0B &= ~((1 << RXEN0) | (1 << TXEN0));
}

void uart_etxint( void )
{
  // Enable TX interrupt
  UCSR0B |= (1 << TXCIE0);
}

void uart_dtxint( void )
{
  // Disable TX interupt
  UCSR0B &= ~(1 << TXCIE0);
}

void uart_erxint( void )
{
  // Enable RX interrupt
  UCSR0B |= (1 << RXCIE0);
}

void uart_drxint( void )
{
  // Disable RX interupt
  UCSR0B &= ~(1 << RXCIE0);
}

void uart_putchar( const char c )
{
  while( (UCSR0A & (1 << UDRE0) ) == 0) {};
  UDR0 = c;
}

char uart_getchar( void )
{
  while( (UCSR0A & (1 <<  RXC0) ) == 0) {};
  return UDR0;
}

unsigned int uart_sync_write( const char* buff, const unsigned int len )
{
  // Loop through buff printing chars to serial
  unsigned int i;
  for( i = 0; i < len; i++ )
  {
    uart_putchar( buff[i] );
  }

  // Make sure everything is out before returning
  while( (UCSR0A & (1 << TXC0) ) == 0) {};
  return i;
}

unsigned int uart_sync_write_to( const char* buff, const unsigned int len, const char endchar )
{
  // Loop through buff printing chars to serial
  unsigned int i;
  for( i = 0; i < len; i++ )
  {
    uart_putchar( buff[i] );
    if( buff[i] == endchar )
    {
      // Make sure everything is out before returning
      while( (UCSR0A & (1 << TXC0) ) == 0) {};
      return i;
    }
  }

  // Make sure everything is out before returning
  while( (UCSR0A & (1 << TXC0) ) == 0) {};
  return i;
}

unsigned int uart_sync_read( char* buff, const unsigned int len )
{
  unsigned int i;
  for( i = 0; i < len; i++ )
  {
    buff[i] = uart_getchar();
  }
  return i;
}

unsigned int uart_sync_read_to( char* buff, const unsigned int len, const char endchar )
{
  unsigned int i;
  for( i = 0; i < len; i++ )
  {
    buff[i] = uart_getchar();
    if( buff[i] == endchar )
    {
      return i;
    }
  }
  return i;
}

