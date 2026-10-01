#include <unistd.h>

#include <i2c.h>
#include <lcd_driver.h>

#define I2C_CLK     100 //in kHz
#define WRITE_ADDR 0x4E      // A2=A1=A0=H, from datasheet
#define PCF8574_ADDR (WRITE_ADDR >> 1) //shifted 7 bit addr

#define LCD_RS (1 << 0)   // P0: 0 = instruction, 1 = data
#define LCD_E  (1 << 2)   // P2
#define LCD_BL (1 << 3)   // P3: backlight which is always on rn
#define LCD_INSTR 0 
#define LCD_DATA LCD_RS
#define HB_SHIFT 4 //HB stands for half-byte
#define HB_MASK 0x0F

#define LCD_SET_DDRAM_ADDR 0x80   // set DDRAM address instruction
#define LCD_ROW0_OFFSET  0x00
#define LCD_ROW1_OFFSET 0x40   // row 1 starts at 0x40
#define LCD_NUM_ROWS  2
#define LCD_NUM_COLS  16

#define LCD_INIT_8BIT    0x3   // function set, 8-bit (upper HB only)
#define LCD_INIT_4BIT    0x2   // function set, switch to 4-bit
#define LCD_FUNC_4BIT_2LINE 0x28
#define LCD_DISPLAY_ON   0x0F
#define LCD_CLEAR        0x01

#define CLEAR_DELAY_CYC 100000

#define LCD_INIT_DELAY_CYC 20000

#define I2C_CLK 100 //in kHz

/**
 * @brief sends one 4-bit half-byte to the LCD with E high -> low
 *
 * @param hb 4-bit value for D4-D7.
 * @param rs  LCD_INSTR/LCD_DATA.
 */
static void lcd_send_hb(uint8_t hb, uint8_t rs) {
    uint8_t out = ((hb & HB_MASK) << HB_SHIFT) | LCD_BL | rs;
    uint8_t buf[2] = { out | LCD_E, out };

    i2c_master_start();
    i2c_master_write(buf, 2, PCF8574_ADDR);
}

/**
 * @brief sends full 8-bit val as two half-bytes in one I2C transmission
 *
 * @param val 8-bit instruction or char
 * @param rs LCD_INSTR/LCD_DATA
 */
static void lcd_send_byte(uint8_t val, uint8_t rs) {
    uint8_t hi = ((val >> HB_SHIFT) << HB_SHIFT) | LCD_BL | rs;
    uint8_t lo = ((val & HB_MASK) << HB_SHIFT) | LCD_BL | rs;
    uint8_t buf[4] = { hi | LCD_E, hi, lo | LCD_E, lo };

    i2c_master_start();
    i2c_master_write(buf, 4, PCF8574_ADDR);
}

/**
 * @brief Initializes I2C and puts the HD44780 into 4-bit, 2-line mode.
 */
void lcd_driver_init() {
    i2c_master_init(I2C_CLK);
	// reset sequence: three 8-bit function sets then switch to 4-bit
    lcd_send_hb(LCD_INIT_8BIT, LCD_INSTR);
    lcd_send_hb(LCD_INIT_8BIT, LCD_INSTR);
    lcd_send_hb(LCD_INIT_8BIT, LCD_INSTR);
    lcd_send_hb(LCD_INIT_4BIT, LCD_INSTR);

    lcd_send_byte(LCD_FUNC_4BIT_2LINE, LCD_INSTR);
    lcd_send_byte(LCD_DISPLAY_ON, LCD_INSTR);

    lcd_clear();
    lcd_clear();
}

/**
 * @brief prints a string at the current cursor position
 *
 * @param input string to print
 *
 * doesn't wrap to the next row so chars past column 15 go off-screen
 */
void lcd_print(char *input){
    if (input == NULL) {
        return;
    }

    while (*input != '\0') {
        lcd_send_byte((uint8_t)*input, LCD_DATA);
        input++;
    }
}

/**
 * @brief moves the LCD cursor to the given row and column
 *
 * @param row row to move to (0 or 1).
 * @param col column to move to (0-15).
 *
 */
void lcd_set_cursor(uint8_t row, uint8_t col){
    if (row >= LCD_NUM_ROWS || col >= LCD_NUM_COLS) {
        return;
    }

    uint8_t offset = (row == 0) ? LCD_ROW0_OFFSET : LCD_ROW1_OFFSET;
    lcd_send_byte(LCD_SET_DDRAM_ADDR | (offset + col), LCD_INSTR);
}

/**
 * @brief clears the display and returns the cursor to (0,0)
 */
void lcd_clear() {
    lcd_send_byte(LCD_CLEAR, LCD_INSTR);

    for (volatile uint32_t i = 0; i < CLEAR_DELAY_CYC; i++);
}