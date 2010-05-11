#define LCD_START_LINE1  0x00     /**< DDRAM address of first char of line 1 */
#define LCD_START_LINE2  0x40     /**< DDRAM address of first char of line 2 */
#define LCD_START_LINE3  0x10    /**< DDRAM address of first char of line 3 */
#define LCD_START_LINE4  0x50    /**< DDRAM address of first char of line 4 */
#define LCD_LINES           4     /**< number of visible lines of the display */
#define LCD_DISP_LENGTH    16     /**< visibles characters per line of the displ */

extern void LCD_init_4 (void);
extern __z void LCD_fprintline (UINT8 line, char __flash* p);
extern __z void LCD_fprintlineEE (UINT8 line, char __eeprom* p);
extern __z void LCD_putBR (char* s);
extern void LCD_softCLR();
extern void LCD_putMETA (UINT8 mode, char c);
//extern void LCD_XY(UINT8 x,UINT8 y);