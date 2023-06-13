/* 
 * File:   Screen_16x2_driver.h
 * Author: Emmanuel Esqueda Rodríguez
 *
 * Created on 2 de junio de 2023, 04:16 PM
 */

#define set 1
#define clean 0

//Minium time of LCD to write the data
#define time_screen (_XTAL_FREQ/133333)

//I'm too lazy to make automatic this part, is only select the bits where each 
//pin is connected, if u want to connect in another way just adjuts the pins 
//position
#define Pin_D0 (PORTDbits.RD0)
#define Pin_D1 (PORTDbits.RD1)
#define Pin_D2 (PORTDbits.RD2)
#define Pin_D3 (PORTDbits.RD3)
#define Pin_D4 (PORTDbits.RD4)
#define Pin_D5 (PORTDbits.RD5)
#define Pin_D6 (PORTDbits.RD6)
#define Pin_D7 (PORTDbits.RD7)
#define Pin_RS (PORTCbits.RC0)
#define Pin_RW (PORTCbits.RC1)
#define Pin_E (PORTCbits.RC2)


void Frame_update();
void Screen_Init();
void Nframe();
void Write_delay();
void Screen_write(unsigned char coor,char Array[]);

unsigned char filas,columnas;
//Superior are the frist line and inferior the second line
unsigned char superior[20];
unsigned char inferior[20];

//Config of interrupts using auto update characters 

/*
 void __interrupt() screen_frames(void){
    //No interrupts while updating the screen
    INTCON = 0;
    Nframe();
    TMR0 = 0;
    //Interrups on
    INTCON = 0b10100000;
    return;
}*/

void Screen_Init(){
    Bus_control = 0;
    Bus_port = 0;
    Bus_control_conf = 0;
    Bus_port_conf = 0;
    //Bus size (8)
	Bus_port = (0x38);
	//No. of lines (2)
    Pin_E = set;
	Pin_E = clean;
	__delay_ms(2);
    //Not blink and not cursor
    Bus_port = 0x0C;
    Pin_E = set;
	Pin_E = clean;
	__delay_ms(2);
    TMR0 = 0;
    OPTION_REG &= 0b01000111;
    INTCON = 0b10100000;
    return;
}

void Nframe(){
	//CGRAM 1st line
	Bus_port = 0x40;
    //Clk command to the display
	Frame_update();
    /*In order to write our character u have to send 5 bits 8 times to make a 
     * complete character. The program works with 5 8bits variable whichs using
     * and masks are ordenated to send. I'll try to explain memory distribution
     * with ASCII art
     * 
     * View of the display:
     *        _  _  _  _  _         _  _  _  _  _  
     *      | *           #|      | *           #|
     *      | *           #|      | *           #|
     *      | *           #|      | *           #|
     *      | *           #|      | *           #|
     *      | *           #|      | *           #|
     *      | *           #|      | *           #|
     *      | *           #|      | *           #|
     *      | *           #|      | *           #|
     *        ¯  ¯  ¯  ¯  ¯         ¯  ¯  ¯  ¯  ¯ 
     *       s[0]        s[4]      s[5]        s[9]
     * The "*" represent the bits saved in superior[0]
     * The "#" represent the bits saved in superior[4]
     * 
     * The first for move 5 to 5 in order to change the memory space.
     * 
     * The second for send with the and mask to select the specific bit in the
     * variable 
     * 
     * 
     */
	for(columnas = 0; columnas != 20;columnas += 5){
		for(filas = 0; filas != 8; filas++){
			if((superior[columnas] >> filas) & 0x01)
				Pin_D4 = set;
			if((superior[(columnas + 1)] >> filas) & 0x01)
				Pin_D3 = set;
			if((superior[(columnas + 2)] >> filas) & 0x01)
				Pin_D2 = set;
			if((superior[(columnas + 3)] >> filas) & 0x01)
				Pin_D1 = set;
			if((superior[(columnas + 4)] >> filas) & 0x01)
				Pin_D0 = set;
			Pin_RS = set;
            //Clk command to the display
			Frame_update();
		}
	}
	//Position to where print the first line 
	Bus_port = (0);
	Pin_D7 = set;
    //Clk command to the display
	Frame_update();
    //Write the first line
	for(filas = 0; filas != 4; filas++){
		Bus_port = (filas);
		Pin_RS = set;
        //Clk command to the display
		Frame_update();
	}
	//CGRAM of 2nd line
	Bus_port = 0x60;
    //Clk command to the display
	Frame_update();
    //For make the same only for the second line
	for(columnas = 0; columnas != 20;columnas += 5){
		for(filas = 0; filas != 8; filas++){
			if((inferior[columnas] >> filas) & 0x01)
				Pin_D4 = set;
			if((inferior[(columnas + 1)] >> filas) & 0x01)
				Pin_D3 = set;
			if((inferior[(columnas + 2)] >> filas) & 0x01)
				Pin_D2 = set;
			if((inferior[(columnas + 3)] >> filas) & 0x01)
				Pin_D1 = set;
			if((inferior[(columnas + 4)] >> filas) & 0x01)
				Pin_D0 = set;
			Pin_RS = set;
			Frame_update();
		}
	}
	//Position to where print
	Bus_port = (0x40);
	Pin_D7 = set;
    //Clk command to the display
	Frame_update();
    //Write the second line
	for(filas = 4; filas != 8; filas ++){
		Bus_port = (filas);
		Pin_RS = set;
        //Clk command to the display
		Frame_update();
	}
    //end
    return;
}

void Frame_update(){
	unsigned char ti;
	Pin_E = set;
	Pin_E = clean;
    //This make the easy way to adjust the time of wait if u want to use a higher frequency
	for (ti = time_screen; ti > 1; ti --);
	Bus_port = 0x00;
	Bus_control = clean;
    return;
}

void Write_delay(){
    unsigned char time;
	Pin_E = set;
	Pin_E = clean;
	for (time = time_screen; time > 1; time --);
	Bus_port = 0;
	Bus_control = 0;
    return;
}

void Screen_write(unsigned char coor, char Array[]){
    unsigned char i;
	//Turn off the interruptions while writing with this function
    INTCON = 0;
    //Set position to write
    Bus_port = coor;
	Pin_D7 = set;
	Write_delay();
	//Escritura
	for(i = 0; Array[i] != 0x60; i++){ 
        //Escritura
        Bus_port = Array[i];
        Pin_RS = set;
        Write_delay();
	}
    //Turn on the interruptions
    INTCON = 0b10100000;
    return;
}