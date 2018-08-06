#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <avr/sleep.h>
#include <util/delay.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "uart.h"

#define CPU_PRESCALE(n) (CLKPR = 0x80, CLKPR = (n))

#define NUM_PINS 10

#define _NOP() do { __asm__ __volatile__ ("nop"); } while (0)

struct pin
{
	//hwstuff - needs definition
	volatile uint8_t *ddr;
	volatile uint8_t *port_reg;
	volatile uint8_t *read_reg;
	int pin_num;
	int pin_id;
};

struct pin pinlist[NUM_PINS] = 
{
	{&DDRD,&PORTD,&PIND,2,0},
	{&DDRD,&PORTD,&PIND,3,1},
	{&DDRD,&PORTD,&PIND,4,2},
	{&DDRD,&PORTD,&PIND,5,3},
	{&DDRD,&PORTD,&PIND,6,4},
	{&DDRD,&PORTD,&PIND,7,5},
	{&DDRC,&PORTC,&PINC,0,6},
	{&DDRC,&PORTC,&PINC,1,7},
	{&DDRC,&PORTC,&PINC,2,8},
	{&DDRC,&PORTC,&PINC,3,9}
};

struct pin reset_pin={&DDRC,&PORTC,&PINC,4,10};
struct pin led_pin={&DDRB,&PORTB,&PINB,5,11};

uint8_t connection_map[NUM_PINS][NUM_PINS] =
{
	{2,0,1,0,0,0,0,0,0,0},
	{0,2,0,0,0,0,0,0,0,0},
	{1,0,2,0,0,0,0,0,0,0},
	{0,0,0,2,0,0,0,0,0,0},
	{0,0,0,0,2,0,0,0,0,0},
	{0,0,0,0,0,2,0,0,0,0},
	{0,0,0,0,0,0,2,0,0,0},
	{0,0,0,0,0,0,0,2,0,0},
	{0,0,0,0,0,0,0,0,2,0},
	{0,0,0,0,0,0,0,0,0,2}
};

/*uint8_t NUM_READ = 0;
uint8_t good_dummy_data[NUM_PINS*(NUM_PINS - 1)] =
{
	0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0
};*/

// Make sure map is symmetric
uint8_t check_map()
{
	uint8_t pass = 1;
	for (unsigned int i = 0; i < NUM_PINS; i++)
	{
		for (unsigned int j = 0; j < NUM_PINS; j++)
		{
			if (connection_map[i][j] != connection_map[j][i])
			{
				printf("Map mismatch. Index[%u][%u] != [%u][%u]\n",i,j,j,i);
				printf("   [%u][%u]: %u. [%u][%u]: %u.\n", i, j, connection_map[i][j], j, i, connection_map[j][i]);
				pass = 0;
			}
		}
	}
	return pass;
}

void configureInput(struct pin * p)
{
	*(p->ddr) &= ~(1<<p->pin_num);
	*(p->port_reg) |= (1<<p->pin_num);
	return;
}

uint8_t readPin(struct pin * p)
{
	_delay_ms(5);
	_NOP();
	// HW Specific - dummied out for now
	uint8_t val = *(p->read_reg) & (1<<p->pin_num);
	val >>= (p->pin_num);
	return val;
}

void setLow(struct pin * p)
{
	*(p->ddr) |= (1<<p->pin_num);
	*(p->port_reg) &= ~(1<<p->pin_num);
	return;
}

void setHigh(struct pin * p)
{
	*(p->ddr) |= (1<<p->pin_num);
	*(p->port_reg) |= (1<<p->pin_num);
	return;
}

int main()
{
  char out_str[200];
  int out_len;
  CPU_PRESCALE( 0 );

  // Initialize the things
  uart_init( );
  uart_start( );
  while(1)
  {

	uint8_t failure_count = 0;
	if (!check_map(connection_map))
		return 0;
	out_len = sprintf(out_str,"Map is symmetric\r\n");
	uart_sync_write(out_str,out_len);


	// Configure all pins as inputs with pull-down resistors
	for (int i = 0; i < NUM_PINS; i++)
	{
		configureInput(&pinlist[i]);
	}
	out_len = sprintf(out_str,"test1\r\n");
	uart_sync_write(out_str,out_len);

	// For each pin in the pinlist, test that pin against all other pins
	for (int i = 0; i < NUM_PINS; i++)
	{
		out_len = sprintf(out_str,"Pin: %d\r\n",i);
		uart_sync_write(out_str,out_len);
		// Set the current pin as an output, and set it high
		setLow(&pinlist[i]);

		// Then, for each other pin, test the input against the map
		for (int j = 0; j < NUM_PINS; j++)
		{
			uint8_t read_val;
			// Skip if self
			if (i == j)
				continue;
	
			read_val = readPin(&pinlist[j]);

			// If connection_map[i][j] is 1, these should be connected
			//  therefore, read_val should be high
			if (connection_map[i][j] == read_val)
			{
				
				out_len = sprintf(out_str,"Failure detected: \r\n");
				uart_sync_write(out_str,out_len);

				out_len = sprintf(out_str,"Pin %d is ", pinlist[i].pin_id);
				uart_sync_write(out_str,out_len);
				if (connection_map[i][j])
				{
					out_len = sprintf(out_str," not ");
					uart_sync_write(out_str,out_len);
				}
				out_len = sprintf(out_str,"connected to Pin %u and should", pinlist[j].pin_id);
				uart_sync_write(out_str,out_len);
				if (!connection_map[i][j])
				{
					out_len = sprintf(out_str,"n't");
					uart_sync_write(out_str,out_len);
				}
				out_len = sprintf(out_str," be\r\n");
				uart_sync_write(out_str,out_len);
				failure_count++ ;
			}
		}
		configureInput(&pinlist[i]);
	}
	out_len = sprintf(out_str,"Failure Count: %u\r\n", failure_count);
	uart_sync_write(out_str,out_len);
	if(failure_count > 0)
	{
		// Light up LED
		setHigh(&led_pin);
		out_len = sprintf(out_str,"\a");
		uart_sync_write(out_str,out_len);
	}
	while(1)
	{
		uint8_t reset_val = readPin(&reset_pin);
		if(!reset_val)
			break;
	}
	setLow(&led_pin);
  }
	
	return 0;
}
