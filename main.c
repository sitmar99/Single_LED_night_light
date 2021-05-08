#define __AVR_ATmega328P__
#define F_CPU 8000000UL

#define BAUD_RATE 9600
#define MYUBRR (F_CPU/16/BAUD_RATE)-1

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

volatile uint16_t sum;
volatile uint8_t nOfSamples;
volatile uint8_t lightLevel;

void USART_Init()
{
    UBRR0H = (unsigned char)((MYUBRR) >> 8);
    UBRR0L = (unsigned char)(MYUBRR);

    UCSR0B = (1 << RXEN0) | (1 << TXEN0);
    UCSR0C = (1 << UCSZ01)| (1 << UCSZ00);
}
void USART_Transmit(uint8_t *data)
{   
    cli();
    for(uint8_t i=0; data[i]!=0; i++)
    {
        while (!(UCSR0A & (1 << UDRE0)));
        UDR0 = data[i];
    }
    sei();
}

void DAC_Init()
{
    ADMUX = (1 << REFS1) | (1 << REFS0) | (1 << ADLAR) | (1 << MUX1) | (1 << MUX0);     //use internal REF & adjust result left & use PORTC3 for DAC
    // ADCSRA = (1 << ADEN) | (1 << ADATE) | (1 << ADIE);   //enable DAC & enable auto trigger & enable DAC interrupt
    ADCSRA = (1 << ADEN);   //enable DAC
    ADCSRA |= (1 << ADPS2);   //set prescaler to 16
    // ADCSRB = (1 << ADTS1) | (1 << ADTS0);   //set trigger to Timer0 compare match A
}
void inline DAC_Start()
{
    ADCSRA |= (1 << ADSC);
}

void Timer0_Init()
{
    TCCR0A = (1 << WGM01);              //clear timer on compare match A
    TCCR0B = (1 << CS02) | (1 << CS00);   //set prescaler to 1024
    TIMSK0 = (1 << OCIE0A);             //enable compare match A interrupt
    TCNT0 = 0;                          //init timer
    OCR0A = 78;                         //set Timer0 freq to ~100 Hz
}

ISR(TIMER0_COMPA_vect)
{
    cli();

    DDRC = 0x00;
    PORTC = 0x00;

    _delay_ms(7);    
    ADCSRA = 0xC4;
    while(ADCSRA & 0x40);
    
    if (lightLevel < 50)
    {
        DDRC = 0xFF;
        PORTC = 0xFF;
    }
    
    sum += ADCH;
    if (++nOfSamples >= 70)
    {
        lightLevel = sum / 70;
        sum = 0;
        nOfSamples = 0;
    }



    sei();
}

// ISR(ADC_vect)
// {
//     cli();
//         uint8_t s[4];
//         uint8_t x = 1;
//         sprintf(s, "%u", x);
//         USART_Transmit(s);
//         USART_Transmit("\n");
//     if (lightLevel < 55)
//     {
//         DDRC = 0xFF;
//         PORTC = 0xFF;
//     }
//
//     sum += ADCH;
//     if (++nOfSamples == 80)
//     {
//         lightLevel = sum/80;
//         sum = 0;
//         nOfSamples = 0;
//     }
//   
//     TCNT0 = 0;
//     sei();
// }

int main()
{
    USART_Init();
    USART_Transmit("\033[2J");      //clear screen
    USART_Transmit("\033[H");       //goto (0;0)
    USART_Transmit("Start \"single_led_night_light\"\n");

    DAC_Init();
    DAC_Start();
    Timer0_Init();
    sei();

    while (1)
    {
        cli();
        uint8_t s[4];
        sprintf(s, "%u", lightLevel);
        USART_Transmit(s);
        USART_Transmit("\n");
        sei();
    }
}