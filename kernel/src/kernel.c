#include <gpio.h>
#include <i2c.h>
#include <adc.h>
#include <printk.h>
#include <uart_polling.h>
#include <unistd.h>
#include <lcd_driver.h>
#include <keypad_driver.h>
 
//calcialted for a baud rate of 115200 with OVER8 = 0
//Gives USARTDIV = 8.6806. 
#define USART_DIV_MANTISSA 8
#define USART_DIV_FRACTION 11
#define USART_BRR ((USART_DIV_MANTISSA << 4) | USART_DIV_FRACTION)

#define LEDG_PORT GPIO_A
#define LEDG_PORTNUM 5

#define LEDR_PORT GPIO_B
#define LEDR_PORTNUM 0

#define BUT1_PORT GPIO_B
#define BUT1_PORTNUM 10

#define BUT2_PORT GPIO_B
#define BUT2_PORTNUM 5

//LS = light sensor
#define LS_CHANNEL   0  //using PA0 rn
#define LS_PRINT_PERIOD 200

#define LCD_NUM_COLS 16

int kernel_main(){ 
  //LEDR(A3 = PB_0)
  gpio_init(LEDR_PORT, LEDR_PORTNUM, MODE_GP_OUTPUT, OUTPUT_PUSH_PULL, OUTPUT_SPEED_LOW, PUPD_NONE, ALT0);
  //set LEDR high for now
  gpio_set(LEDR_PORT, LEDR_PORTNUM);

  //LEDG(D13 = PA_5)
  gpio_init(LEDG_PORT, LEDG_PORTNUM, MODE_GP_OUTPUT, OUTPUT_PUSH_PULL, OUTPUT_SPEED_LOW, PUPD_NONE, ALT0);
  gpio_set(LEDG_PORT, LEDG_PORTNUM);

  //Button 1(D6 = PB_10)
  gpio_init(BUT1_PORT, BUT1_PORTNUM, MODE_INPUT, OUTPUT_PUSH_PULL, OUTPUT_SPEED_LOW, PUPD_PULL_UP, ALT0);
  //Button 2(D4 = PB_5)
  gpio_init(BUT2_PORT, BUT2_PORTNUM, MODE_INPUT, OUTPUT_PUSH_PULL, OUTPUT_SPEED_LOW, PUPD_PULL_UP, ALT0);

  uart_polling_init(USART_BRR); //pause precomputed value for the given baud rate
  keypad_init();
  adc_init();
  lcd_driver_init();

  uint8_t row = 0;
  uint8_t col = 0;

  uint32_t cycles = 0;

  while (1) {
    if(cycles >= LS_PRINT_PERIOD){
      cycles = 0;
      printk("Light Sensor Value: %d\n", adc_read_chan(LS_CHANNEL));
    }else{
      cycles++;
    }

    char key = keypad_read();

    //means no new key presssed
    if(key == '\0') continue;

    char str[] = {key, '\0'};

    //for printing to lcd with the 3 different categories of presses
    if(key == '*'){
      row = !row;
      col = 0;
    } else if (key == '#'){
      lcd_clear();
      lcd_set_cursor(0, 0); 
      //had an issue where # was not correctly clearing so the solution
      //is to just clear twice with a little delay in between
      lcd_clear();
      row = 0;
      col = 0;
    } else {
      lcd_print(str);
      if (col < LCD_NUM_COLS - 1) col++;
    }
    lcd_set_cursor(row, col);
  }

  return 0;
}
