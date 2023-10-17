#include <Arduino.h>
#include "pins.h"

namespace SRAM
{
    namespace tools
    {
        void setAddress( int address )
        {
            // Setup shift register for output
            shiftOut( shiftSerial, shiftClock, MSBFIRST, highByte( address ) );
            shiftOut( shiftSerial, shiftClock, MSBFIRST, lowByte( address ) );

            // Make the shift register output the data set
            digitalWrite( shiftLatch, HIGH );
            digitalWrite( shiftLatch, LOW );
        }

        void setValue( uint8_t value )
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

        uint8_t readValue()
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
    };

    struct ScopedWrite
    {
        ScopedWrite()
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

        void operator()( uint8_t value, int address )
        {
            // Set value to store
            tools::setValue( value );

            // Set memory address value
            tools::setAddress( address );

            // Store data in SRAM
            digitalWrite( sramReadEnable, LOW );
            digitalWrite( sramReadEnable, HIGH );
        }
    };

    struct ScopedRead
    {
        ScopedRead()
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

        uint8_t operator()( int address )
        {
            // Set address from which to read
            tools::setAddress( address );

            // Read the value
            return tools::readValue();
        }
    };
};

bool checkSRAM( int begin, int end, int seed )
{
    randomSeed( seed );

    SRAM::ScopedRead read;

    for( int address = begin; address <= end; address++ )
    {
        const uint8_t value = read( address );

        const uint8_t randomValue = random();


        if( value != randomValue )
        {
            Serial.print( "Address: " );
            Serial.println( address );
            Serial.print( "Value: " );
            Serial.println( value );
            Serial.print( "Random: " );
            Serial.println( randomValue );
            Serial.println();
            return false;
        }
    }

    return true;
}

void fillSRAM( int begin, int end, int seed )
{
    randomSeed( seed );

    SRAM::ScopedWrite write;

    for( int address = begin; address <= end; address++ )
    {
        write( static_cast< uint8_t >( random() ), address );
    }
}

void initialize()
{
    Serial.begin( 115200 );

    pinMode( seed, INPUT );
    pinMode( sramReadEnable, OUTPUT );
    pinMode( shiftSerial, OUTPUT );
    pinMode( shiftLatch, OUTPUT );
    pinMode( shiftClock, OUTPUT );

    digitalWrite( sramReadEnable, HIGH );
    digitalWrite( shiftLatch, LOW );
}

void setup()
{
    initialize();

    // Read data from unconnected pin to get random value for seed
    const int seed = analogRead( A4 );

    constexpr int sramBegin = 0x00;
    constexpr int sramEnd = 0x07FF;
    fillSRAM( sramBegin, sramEnd, seed );

    const bool success = checkSRAM( sramBegin, sramEnd, seed );

    if( success )
    {
        Serial.println( "Successful!" );
    }
    else
    {
        Serial.println( "Error! SRAM does not work properly!" );
    }
}

void loop()
{
}
