#define __AVR_ATmega328P__
#define F_CPU 8000000UL

#define BAUD_RATE 9600
#define MYUBRR (F_CPU/16/BAUD_RATE)-1

#include <avr/io.h>
#include <util/delay.h>

void USART_Init()
{
    UBRR0H = (unsigned char)((MYUBRR) >> 8);
    UBRR0L = (unsigned char)(MYUBRR);

    UCSR0B = (1 << RXEN0) | (1 << TXEN0);
    UCSR0C = (1 << UCSZ01)| (1 << UCSZ00);
}

void USART_Transmit(uint8_t *data)
{    
    for(uint8_t i=0; data[i]!=0; i++)
    {
        while (!(UCSR0A & (1 << UDRE0)));
        UDR0 = data[i];
    }
}

void DAC_Init()
{
    ADMUX = (1 << REFS1) | (1 << REFS0) | (1 << ADLAR) | (1 << MUX1) | (1 << MUX0);     //use AREF & adjust result left & use PORTC3 for DAC
    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);                  //enable DAC & prescaler set to 128
}

void inline DAC_Start()
{
    ADCSRA |= (1 << ADSC);
}

uint8_t inline readLED()
{
    
    uint16_t sum = 0;
    for (uint8_t i=90; i!=0; --i)
    {
        ADCSRA = 0xC7;            // ADCSRA |= (1 << ADSC);          // DAC_Start();
        while(ADCSRA & 0x40);     // while (ADCSRA & (1 << ADSC));
        sum += ADCH;
    }
    return sum/90;
}

int main()
{
    USART_Init();
    DAC_Init();

    while (1)
    {
        // uint8_t x = readLED();
        // uint8_t s[4];
        // sprintf(s, "%u", x);
        // USART_Transmit(s);
        // USART_Transmit("\n");

        DDRC = 0x00;
        PORTC = 0x00;
        if (readLED() < 50)
        {
            DDRC = 0xFF;
            PORTC = 0xFF;
            _delay_ms(1000);
        }
    }
}