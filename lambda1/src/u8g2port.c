#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/logging/log.h>
#include "u8g2port.h"


LOG_MODULE_REGISTER(u8g2z, CONFIG_LOG_DEFAULT_LEVEL);

//struct spi_dt_spec spi = SPI_DT_SPEC_GET(DT_NODELABEL(mcp2515_spi), (SPI_OP_MODE_MASTER|SPI_TRANSFER_MSB|SPI_WORD_SET(8)), 0);
struct spi_dt_spec spi = SPI_DT_SPEC_GET(DT_NODELABEL(ssd1306_ssd1306_128x64_spi), (SPI_OP_MODE_MASTER|SPI_TRANSFER_MSB|SPI_WORD_SET(8)), 0);
//struct gpio_dt_spec gpio_reset = GPIO_DT_SPEC_GET(DT_NODELABEL(ssd1306_ssd1306_128x64_spi), reset_gpios);
//struct gpio_dt_spec gpio_datacmd = GPIO_DT_SPEC_GET(DT_NODELABEL(ssd1306_ssd1306_128x64_spi), data_cmd_gpios);
//struct gpio_dt_spec gpio_cs = GPIO_DT_SPEC_GET(DT_NODELABEL(ssdspi), cs_gpios);


int u8x8initz(){
	if(!spi_is_ready_dt(&spi)){
		LOG_ERR("SPI bus not ready");
	}else{
		LOG_ERR("SPI bus ready");
	}
	//if(gpio_pin_configure_dt(&gpio_cs, 0)){
	//	LOG_ERR("gpio cs config fail");
	//}
/*
	if(gpio_pin_configure_dt(&gpio_reset, GPIO_OUTPUT)){
		LOG_ERR("gpio reset config fail");
	}
	gpio_pin_set_dt(&gpio_reset, 0);

	if(gpio_pin_configure_dt(&gpio_datacmd, GPIO_OUTPUT)){
		LOG_ERR("gpio datacmd config fail");
	}
	gpio_pin_set_dt(&gpio_datacmd, 0);
*/
	return 0;
}

uint8_t u8x8gpioz(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr){
  switch(msg){
    case U8X8_MSG_GPIO_AND_DELAY_INIT:	// called once during init phase of u8g2/u8x8
      break;							// can be used to setup pins
    case U8X8_MSG_DELAY_NANO:			// delay arg_int * 1 nano second
      break;    
    case U8X8_MSG_DELAY_100NANO:		// delay arg_int * 100 nano seconds
      break;
    case U8X8_MSG_DELAY_10MICRO:		// delay arg_int * 10 micro seconds
	k_sleep(K_USEC(arg_int*10));
      break;
    case U8X8_MSG_DELAY_MILLI:			// delay arg_int * 1 milli second
	k_sleep(K_MSEC(arg_int));
      break;
    case U8X8_MSG_DELAY_I2C:				// arg_int is the I2C speed in 100KHz, e.g. 4 = 400 KHz
      break;							// arg_int=1: delay by 5us, arg_int = 4: delay by 1.25us
    case U8X8_MSG_GPIO_D0:				// D0 or SPI clock pin: Output level in arg_int
    //case U8X8_MSG_GPIO_SPI_CLOCK:
      break;
    case U8X8_MSG_GPIO_D1:				// D1 or SPI data pin: Output level in arg_int
    //case U8X8_MSG_GPIO_SPI_DATA:
      break;
    case U8X8_MSG_GPIO_D2:				// D2 pin: Output level in arg_int
      break;
    case U8X8_MSG_GPIO_D3:				// D3 pin: Output level in arg_int
      break;
    case U8X8_MSG_GPIO_D4:				// D4 pin: Output level in arg_int
      break;
    case U8X8_MSG_GPIO_D5:				// D5 pin: Output level in arg_int
      break;
    case U8X8_MSG_GPIO_D6:				// D6 pin: Output level in arg_int
      break;
    case U8X8_MSG_GPIO_D7:				// D7 pin: Output level in arg_int
      break;
    case U8X8_MSG_GPIO_E:				// E/WR pin: Output level in arg_int
      break;
    case U8X8_MSG_GPIO_CS:				// CS (chip select) pin: Output level in arg_int
      break;
    case U8X8_MSG_GPIO_DC:				// DC (data/cmd, A0, register select) pin: Output level in arg_int
/*
	if(arg_int){
	gpio_pin_set_dt(&gpio_datacmd, 1);
	}else{
	gpio_pin_set_dt(&gpio_datacmd, 0);
	}
*/
      break;
    case U8X8_MSG_GPIO_RESET:			// Reset pin: Output level in arg_int
/*
	if(arg_int){
	gpio_pin_set_dt(&gpio_reset, 1);
	}else{
	gpio_pin_set_dt(&gpio_reset, 0);
	}
*/
      break;
    case U8X8_MSG_GPIO_CS1:				// CS1 (chip select) pin: Output level in arg_int
      break;
    case U8X8_MSG_GPIO_CS2:				// CS2 (chip select) pin: Output level in arg_int
      break;
    case U8X8_MSG_GPIO_I2C_CLOCK:		// arg_int=0: Output low at I2C clock pin
      break;							// arg_int=1: Input dir with pullup high for I2C clock pin
    case U8X8_MSG_GPIO_I2C_DATA:			// arg_int=0: Output low at I2C data pin
      break;							// arg_int=1: Input dir with pullup high for I2C data pin
    case U8X8_MSG_GPIO_MENU_SELECT:
      u8x8_SetGPIOResult(u8x8, /* get menu select pin state */ 0);
      break;
    case U8X8_MSG_GPIO_MENU_NEXT:
      u8x8_SetGPIOResult(u8x8, /* get menu next pin state */ 0);
      break;
    case U8X8_MSG_GPIO_MENU_PREV:
      u8x8_SetGPIOResult(u8x8, /* get menu prev pin state */ 0);
      break;
    case U8X8_MSG_GPIO_MENU_HOME:
      u8x8_SetGPIOResult(u8x8, /* get menu home pin state */ 0);
      break;
    default:
      u8x8_SetGPIOResult(u8x8, 1);			// default return value
      break;
  }
  return 1;
}

uint8_t u8x8bytez(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr) {
  uint8_t *data;
//  uint8_t internal_spi_mode; 


	static uint8_t wbuf[100];
	static struct spi_buf spi_wbuf[1];
	spi_wbuf[0].buf = wbuf;
	//spi_wbuf[0].len = 0;
	static struct spi_buf_set spi_set_wbuf = { spi_wbuf, 1 };

  switch(msg) {
    case U8X8_MSG_BYTE_SEND:
      data = (uint8_t *)arg_ptr;
      while( arg_int > 0 ) {
//        SPI.transfer((uint8_t)*data);

	wbuf[spi_wbuf[0].len] = (uint8_t)*data;
	spi_wbuf[0].len++;

        data++;
        arg_int--;
      }  
      break;
    case U8X8_MSG_BYTE_INIT:
//      u8x8_gpio_SetCS(u8x8, u8x8->display_info->chip_disable_level);
//      SPI.begin();
      break;
    case U8X8_MSG_BYTE_SET_DC:
      u8x8_gpio_SetDC(u8x8, arg_int);
      break;
    case U8X8_MSG_BYTE_START_TRANSFER:

	spi_wbuf[0].len = 0;



//      /* SPI mode has to be mapped to the mode of the current controller, at least Uno, Due, 101 have different SPI_MODEx values */
//      internal_spi_mode =  0;
//      switch(u8x8->display_info->spi_mode) {
//        case 0: internal_spi_mode = SPI_MODE0; break;
//        case 1: internal_spi_mode = SPI_MODE1; break;
//        case 2: internal_spi_mode = SPI_MODE2; break;
//        case 3: internal_spi_mode = SPI_MODE3; break;
//      }
//      SPI.beginTransaction(SPISettings(u8x8->display_info->sck_clock_hz, MSBFIRST, internal_spi_mode));
//      u8x8_gpio_SetCS(u8x8, u8x8->display_info->chip_enable_level);  
//      u8x8->gpio_and_delay_cb(u8x8, U8X8_MSG_DELAY_NANO, u8x8->display_info->post_chip_enable_wait_ns, NULL);
      break;
    case U8X8_MSG_BYTE_END_TRANSFER:      

	spi_write_dt(&spi, &spi_set_wbuf);

//      u8x8->gpio_and_delay_cb(u8x8, U8X8_MSG_DELAY_NANO, u8x8->display_info->pre_chip_disable_wait_ns, NULL);
//      u8x8_gpio_SetCS(u8x8, u8x8->display_info->chip_disable_level);
//      SPI.endTransaction();

      break;
    default:
      return 0;
  }  
  return 1;
}
