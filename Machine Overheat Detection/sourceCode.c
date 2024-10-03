#include <reg51.h>

sfr LCD_Port = 0xA0;       // P2 port as Data port
sbit rs = P2^0;            // Register select pin
sbit en = P2^1;            // Enable pin
sbit rd_adc = P3^0;        // Read pin of ADC
sbit wr_adc = P3^1;        // Write pin of ADC
sbit intr_adc = P3^2;      // Interrupt pin of ADC
sbit buzzer = P2^3;        // Buzzer control pin

#define THRESHOLD 30

void delay(unsigned int count);
void LCD_Command(char cmd);
void LCD_data(char char_data);
void LCD_String(char *str);
void LCD_Init(void);
void convert_display(unsigned char);

void delay(unsigned int count) {
    unsigned int i, j;
    for (i = 0; i < count; i++)
        for (j = 0; j < 922; j++); // Adjusted delay value for a 1 ms delay
}

void LCD_Command(char cmd) {
    LCD_Port = (LCD_Port & 0x0F) | (cmd & 0xF0);  // Send upper nibble
    rs = 0;                                       // Command reg.
    en = 1;
    delay(1);
    en = 0;
    delay(1);
    
    LCD_Port = (LCD_Port & 0x0F) | (cmd << 4);    // Send lower nibble
    en = 1;                                       // Enable pulse
    delay(1);
    en = 0;
    delay(1);
}

void LCD_data(char char_data) {
    LCD_Port = (LCD_Port & 0x0F) | (char_data & 0xF0);  // Send upper nibble
    rs = 1;                                             // Data reg.
    en = 1;
    delay(1);
    en = 0;
    delay(1);
    
    LCD_Port = (LCD_Port & 0x0F) | (char_data << 4);     // Send lower nibble
    en = 1;                                             // Enable pulse
    delay(1);
    en = 0;
    delay(1);
}

void LCD_String(char *str) {
    while (*str != '\0') {      // Send each character of string till the NULL
        LCD_data(*str++);       // Call LCD Data write
    }
}

void LCD_Init(void) {
    delay(20);                  // LCD Power ON Initialization time > 15ms
    LCD_Command(0x02);          // 4-bit mode
    LCD_Command(0x28);          // Initialization of 16*2 LCD in 4-bit mode
    LCD_Command(0x0C);          // Display ON, Cursor OFF
    LCD_Command(0x06);          // Auto increment cursor
}

void convert_display(unsigned char value) {
    unsigned char x1, x2, x3;
    LCD_Command(0xc6); // Position the cursor on the LCD
    x1 = (value / 10) + 0x30; // Convert tens digit to ASCII
    x2 = (value % 10) + 0x30; // Convert units digit to ASCII
    x3 = 0xDF; // ASCII code for degree symbol
    LCD_data(x1);
    LCD_data(x2);
    LCD_data(x3);
    LCD_data('C'); // Display 'C' for Celsius
}

void main() {
    unsigned int temperature;
    unsigned char value;

    LCD_Init();
    LCD_String("Temperature");

    P1 = 0xFF;  // Make port 1 as input port
    P2 = 0x00;  // Make port 2 as output port

    intr_adc = 1; // Make INTR pin as input
    rd_adc = 1;   // Set RD pin high
    wr_adc = 1;   // Set WR pin high

    while(1) {
        wr_adc = 0;     // Send low to high pulse on WR pin
        delay(1);       // Wait for ADC conversion (adjust delay as needed)
        wr_adc = 1;

        while(intr_adc == 1); // Wait for end of conversion

        rd_adc = 0;       // Make RD=0 to read the data from ADC
        value = P1;       // Copy ADC data

        convert_display(value);

        temperature = value; // Store the analog value from LM35

        if(temperature > THRESHOLD) {
            buzzer = 1; // Turn on the buzzer if temperature exceeds the threshold
        } else {
            buzzer = 0; // Turn off the buzzer
        }

        delay(100); // Interval between every cycle
        rd_adc = 1; // Make RD=1 for the next cycle
    }
}
