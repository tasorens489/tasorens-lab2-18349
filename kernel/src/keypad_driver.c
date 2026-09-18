#include <unistd.h>

#include <gpio.h>
#include <keypad_driver.h>

/* Columns: outputs, driven low one at a time */

//D10
#define KEYPAD_COL1_PORT GPIO_B
#define KEYPAD_COL1_PIN  6

//D9
#define KEYPAD_COL2_PORT GPIO_C
#define KEYPAD_COL2_PIN  7

//D8
#define KEYPAD_COL3_PORT GPIO_A
#define KEYPAD_COL3_PIN  9

//Rows
//D7
#define KEYPAD_ROW1_PORT GPIO_A
#define KEYPAD_ROW1_PIN  8

//D2
#define KEYPAD_ROW2_PORT GPIO_A
#define KEYPAD_ROW2_PIN  10

//D12
#define KEYPAD_ROW3_PORT GPIO_A
#define KEYPAD_ROW3_PIN  6

//D11
#define KEYPAD_ROW4_PORT GPIO_A
#define KEYPAD_ROW4_PIN  7

static const gpio_port keypad_col_ports[3] = {
        KEYPAD_COL1_PORT, KEYPAD_COL2_PORT, KEYPAD_COL3_PORT
};
static const unsigned int keypad_col_pins[3] = {
    KEYPAD_COL1_PIN, KEYPAD_COL2_PIN, KEYPAD_COL3_PIN
};

static const gpio_port keypad_row_ports[4] = {
    KEYPAD_ROW1_PORT, KEYPAD_ROW2_PORT, KEYPAD_ROW3_PORT, KEYPAD_ROW4_PORT
};
static const unsigned int keypad_row_pins[4] = {
    KEYPAD_ROW1_PIN, KEYPAD_ROW2_PIN, KEYPAD_ROW3_PIN, KEYPAD_ROW4_PIN
};

//we can use this mapping so that we can just print out the char based on row
//and column number
static const char key_out[12] = {'1', '2', '3', '4', '5', '6', '7', '8', '9', '*', '0', '#'};

//busy wait loop for the result to settle
static void gpio_settle(void){
    for(volatile int i = 0; i < 100; i++);
}

//tells us which key was pressed last. Everytime the read
//is called we check if the key press is the same and ignore it
//if that happens
static char last_pressed = '\0';

void keypad_init() {
    //COL are Output and ROWs are the input for the scanning method
    for (int col = 0; col < 3; col++) {
        gpio_init(keypad_col_ports[col], keypad_col_pins[col], MODE_GP_OUTPUT,
                OUTPUT_PUSH_PULL, OUTPUT_SPEED_LOW, PUPD_NONE, ALT0);
        //set all COLs to HIGH so that everything is high in the circuit
        //to detect a button press we pull a COL low and see if any of
        //the rows are pulled low aswell
        gpio_set(keypad_col_ports[col], keypad_col_pins[col]);
    }

     for (int row = 0; row < 4; row++) {
        gpio_init(keypad_row_ports[row], keypad_row_pins[row], MODE_INPUT,
                OUTPUT_PUSH_PULL, OUTPUT_SPEED_LOW, PUPD_PULL_UP, ALT0);
    }
}

//does one scan of the keypad matrix to determine which signals was pressed
char keypad_check(){
    //to scan we loop 3 times and set one of the col low each time
    //then from the row input signals we can tell if any of the keys
    //are pressed
    for(int col = 0; col < 3; col++){
        //pull specific col low 
        gpio_clr(keypad_col_ports[col], keypad_col_pins[col]);
        gpio_settle();

        for(int row = 0; row <4; row++){
            //if that row is low we know a key is pressed
            if(!gpio_read(keypad_row_ports[row], keypad_row_pins[row])){
                gpio_set(keypad_col_ports[col], keypad_col_pins[col]);
                gpio_settle();
                return key_out[row*3 + col];
            }
        }
        //put col back to high
        gpio_set(keypad_col_ports[col], keypad_col_pins[col]);
        gpio_settle();
    }
    return '\0';
}

//for debouncing we need to make sure that the key pressed is unique to last time

char keypad_read() {
    char c = keypad_check();

    //used to wait until we can make sure that enough time has settled
    //for the switch to be stable
    for(int i = 0; i< 100; i++){
        gpio_settle();
    }

    char check = keypad_check();

    //checks that keypad value has settled and that we're not reading the
    //same key twice
    if(c != check){
        return '\0';
    }
    if (c == last_pressed) {
        return '\0';
    }
    
    last_pressed = c;
    return c;

}
