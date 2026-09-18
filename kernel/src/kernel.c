#include <gpio.h>
#include <i2c.h>
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

int kernel_main() {
  //baud is set using a define so parameter doesn't matter
  
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

  uart_polling_init(USART_BRR);
  keypad_init();
  
  //below are 3 while loops for checking the keypad, uart, and buttons for
  //the purposes of checkpoint. This will be modified after checkpoint.
  
  //test for keypad
  printk("keypad test\n");
  while (1) {
    char key = keypad_read();
    if (key != '\0') {
        printk("key: %c\n", key);
    }
  }

  //counter is an arbitrary number of cycles(check that button 1 and 2 work)
  
  //counter so gpio_isn't contantly printing to terminal so often
  /*
  int counter = 0;

  while(1){
    if(counter == 10000){
      int button1 = gpio_read(BUT1_PORT, BUT1_PORTNUM);
      int button2 = gpio_read(BUT2_PORT, BUT2_PORTNUM);
      printk("button 1 = %d   button 2 = %d", button1, button2);
      counter = 0;
    }
    counter++;
  }

  //uart test(simply send the key pressed back: use minicom)
  while(1){
    char c = uart_polling_get_byte();
    uart_polling_put_byte(c);
  }
    */

  return 0;
}
