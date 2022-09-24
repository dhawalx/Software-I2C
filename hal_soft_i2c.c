/************Dependencies *****************************************************/
#include "hal_idkey.h"
#include "hal_board.h"
#include "hal_defs.h"
#include "hal_drivers.h"
#include "hal_mcu.h"
#include "hal_types.h"
#include "osal.h"
#if _print_serial
#include "uart_polled.h"
#endif

/************Global Constants & Variables *************************************/
unsigned char eep_buf;        // data buffer
unsigned int adr_cnt;         // address counter
unsigned char ch_cnt;         // character counter inside  strings
uint32_t readIDpin[8];

/************Functions Prototypes********************************************************/
static void ini_i2c(void);
static void i2c_start(void);
static void i2c_stop(void);
static void nack_mcu(void);
static uint8_t ack(void);
static void i2c_wr(void);
static unsigned char i2c_rd(void);
static void i2c_rndwr(unsigned int eep_adr, unsigned char eep_data);
static void i2c_rndrd(unsigned int eep_adr, unsigned char *dst);

/************Functions********************************************************/
/*
   I2C Init SDA SCL pin config as output drives high
*/
static void ini_i2c(void) 
{
  //config SDA SCL as Output
  SDA_Output();
  SCL_Output();

  //drives SDA and SCL high
  SDA_Out_High();  // Drive SDA High
  SCL_Out_High();  // Drive SCL High 
}

/*
   I2C Start Condition
*/
static void i2c_start(void) 
{
  // Initial state of the I2C bus
  SDA_Output();   // SDA as Output
  SDA_Out_High(); // Drive SDA High
  SCL_Out_High(); // Drive SCL High
  dlyusec(7);     // Delay of 7uS

  //High to Low transition of SDA
  SDA_Out_Low();  // Drive SDA Low  
  dlyusec(5);     // Delay of 5uS
  SCL_Out_Low();  // Drive SCL Low
  dlyusec(5);     // Delay of 5uS
}

/*
   I2C Stop Condition 
*/
static void i2c_stop(void) 
{
  //SDA Low to High Transition
  SDA_Output();   // SDA as Output
  SCL_Out_Low();  // Drive SCL Low 
  SDA_Out_Low();  // Drive SDA Low
  dlyusec(2);     // Delay of 2uS
  SCL_Out_High(); // Drive SCL High
  dlyusec(6);     // Delay of 6uS
  SDA_Out_High(); // Drive SDA High 
  dlyusec(4);     // Delay of 6uS
}

/*
   I2C NACK Condition 
*/
static void nack_mcu(void) 
{
  SDA_Out_High(); // Drive SDA High
  dlyusec(3);     // Delay of 3 uS
  SCL_Out_High(); // Drive SCL High
  dlyusec(6);     // Delay of 6 uS
  SCL_Out_Low();  // Drive SCL Low
  dlyusec(2);     // Delay of 2 uS
  SDA_Out_Low();  // Drive SDA Low
}

/*
   I2C ACK Condition 
*/
static uint8_t ack(void) 
{
  uint8_t ret;
  //Config SDA as Input and 
  SDA_Input();        // Rise-up SDA and program it as input
  dlyusec(3);         // Delay of 3 uS
  ret = SDA_Read();   // To read the ACK from the memory
  SCL_Out_High();     // Drive SCL High
  dlyusec(5);         // Delay of 5 uS
  SCL_Out_Low();      // Drive SCL Low
  dlyusec(25);        // Delay of 25 uS
  return ret;
}

/*
   I2C Write 8 Bit Data  
*/
static void i2c_wr(void) 
{
  unsigned char bit_count = 0; // bit counter for the 8bit
  unsigned char temp = 0;

  //Send Bit one by one
  while (bit_count < 8) 
  {
    //Shift 7 bit position eep_buf and send out each bit one by one
    SDA_Output();              // SDA as Output
    dlyusec(3);                // Delay of 3 uS     
    temp = eep_buf >> 7;       // Send MSB First
    if (temp) 
    {
      SDA_Out_High();          // Drive SDA High 
    } 
    else
    {  
      SDA_Out_Low();           // Drive SDA Low
    }
    dlyusec(2);                // Delay of 2 uS
    SCL_Out_High();            // Drive SCL High
    dlyusec(5);                // Delay of 5 uS
    SCL_Out_Low();             // Drive SCL Low

    // shift left 1bit for remove msb bit from eep's data buf
    eep_buf = eep_buf << 1;    
    bit_count++;
    dlyusec(2);               // Delay of 2 uS    
  }
}

/*
   I2C Read 8 Bit Data  
*/
static unsigned char i2c_rd(void) 
{
  // bit counter the 8b streaming
  unsigned char bit_count = 0; 

  // Set SDA P0_2 pin as input
  SDA_Input();                

  //Process each 8 bit data one by one 
  while (bit_count < 8) 
  {
    // MSB First is received so shift left 1 bit eep_buf to shift out rx data 
    eep_buf = eep_buf << 1; 

    dlyusec(4);             // Delay of 4uS
    SCL_Out_High();         // rise-up SCL
    
    //Read SDA pin state
    readIDpin[bit_count] = SDA_Read();

    //If Pin is high set bit else it stays zero
    if (readIDpin[bit_count] & SDAPIN_PD3) {
      eep_buf |= (1 << 0);
    }
    dlyusec(4);             // Delay of 4 uS
    SCL_Out_Low();          // Drive SCL Low
    dlyusec(2);             // Delay of 2 uS
    bit_count++;            // Increase Bit Counter 
  }

  SDA_Output();             // SDA as Output
  return eep_buf;           // Return data buf
}

/*
   I2C Random Write Operation
   START+SLAVE_ADDR+WRITE_BIT+ACK+ADDR+ACK+DATA+ACK+STOP 
*/
static void i2c_rndwr(unsigned int eep_adr, unsigned char eep_data) 
{
  i2c_start();              // I2C START 
  eep_buf = DEV_ADR_WR;     // Device Address + Write Bit
  i2c_wr();                 // write first slave adr + write_bit
  ack();                    // I2C ACK
  
  // Write the low_byte of the address
  eep_buf = eep_adr & 0xff; 
  i2c_wr();                 // I2C Write Byte
  ack();                    // I2C ACK
  eep_buf = eep_data;       // Data to be writen 
  i2c_wr();                 // I2C Write the data_character
  ack();                    // I2C ACK  
  i2c_stop();               // I2C STOP command
  delayms(18);              // Delay of 18 mS  
}

/*
   I2C Random Read Operation
   START+SLAVE_ADDR+WRITE_BIT+ACK+ADDR+ACK+REPEATED START+ADDR+READBIT+ACK+READ_DATA+NACK+STOP 
*/
static void i2c_rndrd(unsigned int eep_adr, unsigned char *dst) 
{
  i2c_start();              // START command
  eep_buf = DEV_ADR_WR;
  i2c_wr();                 // write slave address + write_bit
  ack();
  eep_buf = eep_adr & 0xff;
  i2c_wr();                 // write the  low_byte of the adr
  ack();
  i2c_start();              // REPEATED START condition
  eep_buf = DEV_ADR_RD;
  i2c_wr();                 // change the direction of the trsf
  ack();
  *dst = i2c_rd();          // store the result in "dst"(<-eep_buf)
  nack_mcu();               // send a NACK from MCU to the memory
  i2c_stop();               // finally , STOP command
}
