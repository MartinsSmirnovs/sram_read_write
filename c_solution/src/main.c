#include <Arduino.h>

enum Pin
{
    seedIn = 18,
    sramReadEnable = 17,
    shiftSerial = 16,
    shiftLatch = 15,
    shiftClock = 14,
    sramIO0 = 2,
    sramIO1 = 3,
    sramIO2 = 4,
    sramIO3 = 5,
    sramIO4 = 6,
    sramIO5 = 7,
    sramIO6 = 8,
    sramIO7 = 9,
};

void _sramSetAddress( int address )
{
    // Setup shift register for output
    shiftOut( shiftSerial, shiftClock, MSBFIRST, highByte( address ) );
    shiftOut( shiftSerial, shiftClock, MSBFIRST, lowByte( address ) );

    // Make the shift register output the data set
    digitalWrite( shiftLatch, HIGH );
    digitalWrite( shiftLatch, LOW );
}

void _sramSetValue( uint8_t value )
{
    digitalWrite( sramIO0, bitRead( value, 0 ) );
    digitalWrite( sramIO1, bitRead( value, 1 ) );
    digitalWrite( sramIO2, bitRead( value, 2 ) );
    digitalWrite( sramIO3, bitRead( value, 3 ) );
    digitalWrite( sramIO4, bitRead( value, 4 ) );
    digitalWrite( sramIO5, bitRead( value, 5 ) );
    digitalWrite( sramIO6, bitRead( value, 6 ) );
    digitalWrite( sramIO7, bitRead( value, 7 ) );
}

uint8_t _sramReadValue()
{
    uint8_t value = 0;

    bitWrite( value, 0, digitalRead( sramIO0 ) );
    bitWrite( value, 1, digitalRead( sramIO1 ) );
    bitWrite( value, 2, digitalRead( sramIO2 ) );
    bitWrite( value, 3, digitalRead( sramIO3 ) );
    bitWrite( value, 4, digitalRead( sramIO4 ) );
    bitWrite( value, 5, digitalRead( sramIO5 ) );
    bitWrite( value, 6, digitalRead( sramIO6 ) );
    bitWrite( value, 7, digitalRead( sramIO7 ) );

    return value;
}

void sramWritePrepare()
{
    pinMode( sramIO0, OUTPUT );
    pinMode( sramIO1, OUTPUT );
    pinMode( sramIO2, OUTPUT );
    pinMode( sramIO3, OUTPUT );
    pinMode( sramIO4, OUTPUT );
    pinMode( sramIO5, OUTPUT );
    pinMode( sramIO6, OUTPUT );
    pinMode( sramIO7, OUTPUT );
}

void sramWrite( uint8_t value, int address )
{
    // Set value to store
    _sramSetValue( value );

    // Set memory address value
    _sramSetAddress( address );

    // Store data in SRAM
    digitalWrite( sramReadEnable, LOW );
    digitalWrite( sramReadEnable, HIGH );
}

void sramReadPrepare()
{
    pinMode( sramIO0, INPUT );
    pinMode( sramIO1, INPUT );
    pinMode( sramIO2, INPUT );
    pinMode( sramIO3, INPUT );
    pinMode( sramIO4, INPUT );
    pinMode( sramIO5, INPUT );
    pinMode( sramIO6, INPUT );
    pinMode( sramIO7, INPUT );
}

uint8_t sramRead( int address )
{
    // Set address from which to read
    _sramSetAddress( address );

    // Read the value
    return _sramReadValue();
}

bool checkSRAM( int begin, int end, int seed )
{
    sramReadPrepare();

    for( int address = begin; address <= end; address++ )
    {
        const uint8_t value = sramRead( address );
        const uint8_t randomValue = seed++;

        if( value != randomValue )
        {
            return false;
        }
    }

    return true;
}

void fillSRAM( int begin, int end, int seed )
{
    sramWritePrepare();

    for( int address = begin; address <= end; address++ )
    {
        const uint8_t randomValue = seed++;
        sramWrite( randomValue, address );
    }
}

void initialize()
{
    pinMode( seedIn, INPUT );
    pinMode( sramReadEnable, OUTPUT );
    pinMode( shiftSerial, OUTPUT );
    pinMode( shiftLatch, OUTPUT );
    pinMode( shiftClock, OUTPUT );
    pinMode( LED_BUILTIN, OUTPUT );

    digitalWrite( sramReadEnable, HIGH );
    digitalWrite( shiftLatch, LOW );
    digitalWrite( LED_BUILTIN, LOW );
}

void setup()
{
    initialize();

    // Read data from unconnected pin to get random value for seed
    const int seed = analogRead( seedIn );

    const int sramBegin = 0x00;
    const int sramEnd = 0x07FF;
    fillSRAM( sramBegin, sramEnd, seed );

    const bool result = checkSRAM( sramBegin, sramEnd, seed );

    // Output the result using built-in LED
    // Success - the LED is turned on
    // Failure - the LED is turned off
    digitalWrite( LED_BUILTIN, result );
}

void loop()
{
}
