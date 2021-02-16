#include "stm32l1xx.h"

#include "stm32l1xx_ll_gpio.h"
#include "stm32l1xx_ll_pwr.h"
#include "stm32l1xx_ll_rcc.h"
#include "stm32l1xx_ll_bus.h"
#include "stm32l1xx_ll_utils.h"
#include "stm32l1xx_ll_system.h"
#include "stm32l1xx_ll_tim.h"
#include "stm32l1xx_ll_spi.h"

#include <stdbool.h>

//for Random
#include "stm32l1xx_ll_rtc.h"
#include "stm32l1xx_ll_usart.h"
#include <string.h>
#include "stdlib.h"

#define POS_X 0
#define NEG_X 1
#define POS_Z 2
#define NEG_Z 3
#define POS_Y 4
#define NEG_Y 5

#define BUTTON_PIN LL_GPIO_PIN_8
#define RED_LED LL_GPIO_PIN_5
#define GREEN_LED LL_GPIO_PIN_7
#define SS LL_GPIO_PIN_4
#define SCK LL_GPIO_PIN_5
#define MOSI LL_GPIO_PIN_12

#define TOTAL_EFFECTS 1
#define RAIN 0

#define RAIN_TIME 260
#define CLOCK_TIME 500

uint8_t cube[4][4];
uint8_t currentEffect;

uint16_t timer;

uint64_t randomTimer;

bool autoRotate = true;
uint64_t lastEffectChange = 0;
uint32_t effectDuration = 5000;
uint64_t current_time;												//In Progress : current_time = millis()

bool loading;
void rtc_config(void);
void SystemClock_Config(void);
void GPIO_Config(void);
void SPI_Config(void);

void clearCube(void);
void rain(void);


void renderCube(void);
void shift(uint8_t dir);
void setVoxel(uint8_t , uint8_t , uint8_t );
void setPlane(uint8_t , uint8_t );
bool getVoxel(uint8_t , uint8_t , uint8_t );
void clearVoxel(uint8_t , uint8_t , uint8_t );
void drawCube(uint8_t , uint8_t , uint8_t , uint8_t );
void lightCube(void);
void Select_Effect(uint8_t currentEffect);
void Active(uint8_t current_time,uint8_t);
int main()
{
	loading = true;
	randomTimer = 0;
	currentEffect = RAIN;
	
	SystemClock_Config();
	GPIO_Config();
	SPI_Config();
	
	//randomSeed(analogRead(0))
	
	srand((LL_RTC_DATE_Get(RTC))*(LL_RTC_TIME_Get(RTC)));//seed for random
	srand(LL_RTC_TIME_Get(RTC));
	
	
	LL_GPIO_SetOutputPin(GPIOB, GREEN_LED);
	
	while(true)
	{
		/*
		current_time = randomTimer%1000;
		randomTimer++;
		
		Active(current_time,randomTimer);
		
		Select_Effect(currentEffect);
		
		renderCube();
		*/
		
		LL_GPIO_ResetOutputPin(GPIOA, SS);
		
		LL_SPI_TransmitData16(SPI1, 0xF);
		
		LL_GPIO_SetOutputPin(GPIOA, SS);
		/*
		uint8_t i, j;
		for(i = 0; i < 8; ++i)
		{
			LL_GPIO_ResetOutputPin(GPIOA, SS);
			LL_SPI_TransmitData16(SPI1, 0x01<<i);
			for(j = 0; j < 8; ++j)
			{
				LL_SPI_TransmitData8(SPI1, cube[i][j]);
				LL_mDelay(500);
			}
			LL_GPIO_SetOutputPin(GPIOA, SS);
		}*/
		
	}
	
	
}
void Active(uint8_t current_time,uint8_t randomTimer)
{
	if(!LL_GPIO_IsInputPinSet(GPIOB, BUTTON_PIN) )
		{
			//lastEffectChange =                      //millis()
			clearCube();
			loading = true;
			timer = 0;
			currentEffect++;
			if(currentEffect == TOTAL_EFFECTS)
			{
				currentEffect = 0;		
			}
			//randomSeed(randomTimer);
			srand((LL_RTC_DATE_Get(RTC))*(LL_RTC_TIME_Get(RTC)));//seed for random
			srand(LL_RTC_TIME_Get(RTC));
			
			randomTimer = 0;
			LL_GPIO_SetOutputPin(GPIOB, RED_LED);
			LL_GPIO_ResetOutputPin(GPIOB, GREEN_LED);
			LL_mDelay(500);
			LL_GPIO_TogglePin(GPIOB, RED_LED);
			LL_GPIO_TogglePin(GPIOB, GREEN_LED);
		}
		
}

void Select_Effect(uint8_t currentEffect)
{
	switch (currentEffect)
		{
			case RAIN : rain(); break;
			
			default : rain();
		}
}

void GPIO_Config(void)
{
	LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);
	LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB);
	LL_GPIO_InitTypeDef io;
	
	//SPI GPIO Config//
	io.Mode = LL_GPIO_MODE_ALTERNATE;
	io.Pin = SS | SCK | MISO | MOSI;
	io.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
	io.Pull = LL_GPIO_PULL_NO;
	io.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
	io.Alternate = LL_GPIO_AF_5;
	
	LL_GPIO_Init(GPIOA, &io);
	//SPI GPIO Config//
	
	//BUTTON GPIO Config//
	io.Mode = LL_GPIO_MODE_INPUT;
	io.Pin = BUTTON_PIN;
	io.Pull = LL_GPIO_PULL_UP;
	
	LL_GPIO_Init(GPIOB, &io);
	//BUTTON GPIO Config//
	
	//RED and GREEN LED Config//
	io.Mode = LL_GPIO_MODE_OUTPUT;
	io.Pin = RED_LED | GREEN_LED;
	io.Pull = LL_GPIO_PULL_NO;
	
	LL_GPIO_Init(GPIOB, &io);
	//RED and GREEN LED Config//
}

void SPI_Config(void)
{
	LL_AHB1_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_SPI1);
	LL_SPI_InitTypeDef spi;
	
	spi.TransferDirection = LL_SPI_FULL_DUPLEX;
	spi.Mode = LL_SPI_MODE_MASTER;
	spi.DataWidth = LL_SPI_DATAWIDTH_8BIT;
	spi.ClockPolarity = LL_SPI_POLARITY_LOW;
	spi.ClockPhase = LL_SPI_PHASE_2EDGE;
	spi.NSS = LL_SPI_NSS_SOFT;
	spi.BaudRate = LL_SPI_BAUDRATEPRESCALER_DIV2;
	spi.BitOrder = LL_SPI_MSB_FIRST;
	spi.CRCCalculation = LL_SPI_CRCCALCULATION_DISABLE;
	spi.CRCPoly = 10;
	
	LL_SPI_Init(SPI1, &spi);
	
	LL_SPI_Enable(SPI1);
}

void renderCube(void)
{
	uint8_t i, j;
	for(i = 0; i < 4; ++i)
	{
		LL_GPIO_ResetOutputPin(GPIOA, SS);
		LL_SPI_TransmitData8(SPI1, 0x01<<i);
		for(j = 0; j < 4; ++j)
		{
			LL_SPI_TransmitData16(SPI1, cube[i][j]);
		}
		LL_GPIO_SetOutputPin(GPIOA, SS);
	}
}

void rain(void)
{
	uint8_t i;
	srand((LL_RTC_DATE_Get(RTC))*(LL_RTC_TIME_Get(RTC)));//seed for random
	srand(LL_RTC_TIME_Get(RTC));
	
	if(loading)
	{
		clearCube();
		loading = false;
	}
	timer++;
	if(timer > RAIN_TIME)
	{
		timer = 0;
		shift(NEG_Y);
		uint8_t numDrops;     //= random(0, 5);
		numDrops = rand()%4;
		
		for(i = 0; i < numDrops; ++i)
		{
			//setVoxel(random(0, 8), 7, random(0, 8))
			
			setVoxel(rand()%4,3,rand()%4);
		}
	}
}


void setVoxel(uint8_t x, uint8_t y, uint8_t z) 
{
  cube[3 - y][3 - z] |= (0x01 << x);
}

void shift(uint8_t dir) 
{
  if(dir == POS_X) 
	{
    for(uint8_t y = 0; y < 4; y++) 
		{
      for(uint8_t z = 0; z < 4; z++) 
			{
        cube[y][z] = cube[y][z] << 1;
      }
    }
  } 
	else if(dir == NEG_X) 
	{
    for (uint8_t y = 0; y < 4; y++) 
		{
      for (uint8_t z = 0; z < 4; z++) 
			{
        cube[y][z] = cube[y][z] >> 1;
      }
    }
  } 
	else if(dir == POS_Y) 
	{
    for (uint8_t y = 1; y < 4; y++) 
		{
      for (uint8_t z = 0; z < 4; z++) 
			{
        cube[y - 1][z] = cube[y][z];
      }
    }
    for(uint8_t i = 0; i < 4; i++) 
		{
      cube[3][i] = 0;
    }
  } 
	else if(dir == NEG_Y) 
	{
    for(uint8_t y = 4; y > 0; y--) 
		{
      for(uint8_t z = 0; z < 4; z++) 
			{
        cube[y][z] = cube[y - 1][z];
      }
    }
    for(uint8_t i = 0; i < 4; i++) 
		{
      cube[0][i] = 0;
    }
  } 
	else if (dir == POS_Z) 
	{
    for (uint8_t y = 0; y < 4; y++) 
		{
      for (uint8_t z = 1; z < 4; z++) 
			{
        cube[y][z - 1] = cube[y][z];
      }
    }
    for (uint8_t i = 0; i < 4; i++) 
		{
      cube[i][3] = 0;
    }
  } 
	else if (dir == NEG_Z) 
	{
    for (uint8_t y = 0; y < 4; y++) 
		{
      for (uint8_t z = 3; z > 0; z--) 
			{
        cube[y][z] = cube[y][z - 1];
      }
    }
    for (uint8_t i = 0; i < 4; i++) 
		{
      cube[i][0] = 0;
    }
  }
}

void clearCube() 
{
  for (uint8_t i = 0; i < 4; i++) 
	{
    for (uint8_t j = 0; j < 4; j++) 
		{
      cube[i][j] = 0;
    }
  }
}

void rtc_config()
{
	LL_RTC_InitTypeDef RTC_InitStruct;
	LL_RTC_TimeTypeDef RTC_TimeStruct;
	

	LL_RTC_DateTypeDef RTC_DateStruct;
	LL_RTC_AlarmTypeDef RTC_AlarmStruct;
	
	LL_RCC_SetRTCClockSource(LL_RCC_RTC_CLKSOURCE_LSE);
	LL_RCC_EnableRTC();
	
	//test = LL_RCC_IsEnabledRTC();
	
	RTC_InitStruct.HourFormat = LL_RTC_HOURFORMAT_24HOUR;
	RTC_InitStruct.AsynchPrescaler = 127;
	RTC_InitStruct.SynchPrescaler = 255;
	
	LL_RTC_Init(RTC, &RTC_InitStruct);

	RTC_TimeStruct.TimeFormat = LL_RTC_TIME_FORMAT_AM_OR_24;
	RTC_TimeStruct.Hours      = 0U;
  RTC_TimeStruct.Minutes    = 0U;
  RTC_TimeStruct.Seconds    = 0U;
	
	LL_RTC_TIME_StructInit(&RTC_TimeStruct);
	
	RTC_DateStruct.WeekDay = LL_RTC_WEEKDAY_MONDAY;
  RTC_DateStruct.Day     = 1U;
  RTC_DateStruct.Month   = LL_RTC_MONTH_JANUARY;
  RTC_DateStruct.Year    = 0U;
	
	LL_RTC_DATE_Init(RTC, LL_RTC_FORMAT_BCD, &RTC_DateStruct);
	
	RTC_AlarmStruct.AlarmTime.Hours = 0;
	RTC_AlarmStruct.AlarmTime.Minutes = 0;
	RTC_AlarmStruct.AlarmTime.Seconds = 10;
	RTC_AlarmStruct.AlarmDateWeekDaySel = LL_RTC_ALMA_DATEWEEKDAYSEL_DATE;
	RTC_AlarmStruct.AlarmDateWeekDay = 1;
	
	LL_RTC_ALMA_Init(RTC, LL_RTC_FORMAT_BCD, &RTC_AlarmStruct);
	
	NVIC_SetPriority(RTC_Alarm_IRQn, 0);
	NVIC_EnableIRQ(RTC_Alarm_IRQn);	
}

void SystemClock_Config(void)
{
  /* Enable ACC64 access and set FLASH latency */ 
  LL_FLASH_Enable64bitAccess();; 
  LL_FLASH_SetLatency(LL_FLASH_LATENCY_1);

  /* Set Voltage scale1 as MCU will run at 32MHz */
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);
  LL_PWR_SetRegulVoltageScaling(LL_PWR_REGU_VOLTAGE_SCALE1);
  
  /* Poll VOSF bit of in PWR_CSR. Wait until it is reset to 0 */
  while (LL_PWR_IsActiveFlag_VOSF() != 0)
  {
  };
  
  /* Enable HSI if not already activated*/
  if (LL_RCC_HSI_IsReady() == 0)
  {
    /* HSI configuration and activation */
    LL_RCC_HSI_Enable();
    while(LL_RCC_HSI_IsReady() != 1)
    {
    };
  }
  
	
  /* Main PLL configuration and activation */
  LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_HSI, LL_RCC_PLL_MUL_6, LL_RCC_PLL_DIV_3);

  LL_RCC_PLL_Enable();
  while(LL_RCC_PLL_IsReady() != 1)
  {
  };
  
  /* Sysclk activation on the main PLL */
  LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
  LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);
  while(LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL)
  {
  };
  
  /* Set APB1 & APB2 prescaler*/
  LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);
  LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_1);

  /* Set systick to 1ms in using frequency set to 32MHz                             */
  /* This frequency can be calculated through LL RCC macro                          */
  /* ex: __LL_RCC_CALC_PLLCLK_FREQ (HSI_VALUE, LL_RCC_PLL_MUL_6, LL_RCC_PLL_DIV_3); */
  LL_Init1msTick(32000000);
  
  /* Update CMSIS variable (which can be updated also through SystemCoreClockUpdate function) */
  LL_SetSystemCoreClock(32000000);
}