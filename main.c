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

#define XAXIS 0
#define YAXIS 1
#define ZAXIS 2

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
#define MISO LL_GPIO_PIN_6
#define MOSI LL_GPIO_PIN_7

#define TOTAL_EFFECTS 8
#define RAIN 0
#define PLANE_BOING 1
#define SEND_VOXELS 2
#define WOOP_WOOP 3
#define CUBE_JUMP 4
#define GLOW 5
#define TEXT 6
#define LIT 7

#define RAIN_TIME 260
#define PLANE_BOING_TIME 220
#define SEND_VOXELS_TIME 140
#define WOOP_WOOP_TIME 350
#define CUBE_JUMP_TIME 200
#define GLOW_TIME 8
#define TEXT_TIME 300
#define CLOCK_TIME 500

uint8_t characters[10][8] = {
  {0x3C, 0x42, 0x42, 0x42, 0x42, 0x42, 0x42, 0x3C}, //0
  {0x10, 0x18, 0x14, 0x10, 0x10, 0x10, 0x10, 0x3C}, //1
  {0x3C, 0x42, 0x40, 0x40, 0x3C, 0x02, 0x02, 0x7E}, //2
  {0x3C, 0x40, 0x40, 0x3C, 0x40, 0x40, 0x42, 0x3C}, //3
  {0x22, 0x22, 0x22, 0x22, 0x7E, 0x20, 0x20, 0x20}, //4
  {0x7E, 0x02, 0x02, 0x3E, 0x40, 0x40, 0x42, 0x3C}, //5
  {0x3C, 0x02, 0x02, 0x3E, 0x42, 0x42, 0x42, 0x3C}, //6
  {0x3C, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40}, //7
  {0x3C, 0x42, 0x42, 0x3C, 0x42, 0x42, 0x42, 0x3C}, //8
  {0x3C, 0x42, 0x42, 0x42, 0x3C, 0x40, 0x40, 0x3C}, //9
};

uint8_t cube[8][8];
uint8_t currentEffect;

uint16_t timer;

uint64_t randomTimer;

bool autoRotate = true;
uint64_t lastEffectChange = 0;
uint32_t effectDuration = 5000;

bool loading;
void rtc_config(void);
void SystemClock_Config(void);
void GPIO_Config(void);
void SPI_Config(void);

void clearCube(void);
void rain(void);
void planeBoing(void);
void sendVoxels(void);
void woopWoop(void);
void cubeJump(void);
void glow(void);
void text(char string[], uint8_t );
void lit(void);
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
	
	//srand((LL_RTC_DATE_Get(RTC))*(LL_RTC_TIME_Get(RTC)));//seed for random
	//srand(LL_RTC_TIME_Get(RTC));
	
	
	LL_GPIO_SetOutputPin(GPIOB, GREEN_LED);
	
	while(true)
	{
		uint64_t current_time;												//In Progress : current_time = millis()
		randomTimer++;
		
		Active(current_time,randomTimer);
		
		Select_Effect(currentEffect);
		
		renderCube();
	}
	
	
}
void Active(uint8_t current_time,uint8_t randomTimer)
{
	if(!LL_GPIO_IsInputPinSet(GPIOB, BUTTON_PIN) || (current_time - lastEffectChange >= effectDuration && autoRotate))
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
			//srand((LL_RTC_DATE_Get(RTC))*(LL_RTC_TIME_Get(RTC)));//seed for random
			//srand(LL_RTC_TIME_Get(RTC));
			
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
			case PLANE_BOING : planeBoing(); break;
			case SEND_VOXELS : sendVoxels(); break;
			case WOOP_WOOP : woopWoop(); break;
			case CUBE_JUMP : cubeJump(); break;
			case GLOW : glow(); break;
			case TEXT : text("0123456789", 10); break;
			case LIT : lit(); break;
			
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
}

void renderCube(void)
{
	uint8_t i, j;
	for(i = 0; i < 8; ++i)
	{
		LL_GPIO_ResetOutputPin(GPIOA, SS);
		LL_SPI_TransmitData8(SPI1, 0x01<<i);
		for(j = 0; j < 8; ++j)
		{
			LL_SPI_TransmitData8(SPI1, cube[i][j]);
		}
		LL_GPIO_SetOutputPin(GPIOA, SS);
	}
}

void rain(void)
{
	uint8_t i;
	//srand((LL_RTC_DATE_Get(RTC))*(LL_RTC_TIME_Get(RTC)));//seed for random
	//srand(LL_RTC_TIME_Get(RTC));
	
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
		//numDrops = rand()%5;
		
		for(i = 0; i < numDrops; ++i)
		{
			//setVoxel(random(0, 8), 7, random(0, 8))
			
			//setVoxel(rand()%8,7,rand()%8)
		}
	}
}

void planeBoing(void)
{
	uint8_t planePosition = 0;
	uint8_t planeDirection = 0;
	bool looped = false;
	
	//srand((LL_RTC_DATE_Get(RTC))*(LL_RTC_TIME_Get(RTC)));//seed for random
	//srand(LL_RTC_TIME_Get(RTC));
	
	if(loading) 
	{
    clearCube();
    uint8_t axis; 				//= random(0, 3);
		//axis = rand()%3;
		
    //planePosition = random(0, 2) * 7;
		//planePosition = rand()%2 * 7;
		
    setPlane(axis, planePosition);
    if(axis == XAXIS) 
		{
			if(planePosition == 0) 
				{
					planeDirection = POS_X;
				} 
				else 
				{
					planeDirection = NEG_X;
				}
		} 
		else if(axis == YAXIS) 
		{
      if(planePosition == 0) 
			{
        planeDirection = POS_Y;
      } 
			else 
			{
        planeDirection = NEG_Y;
      }
    } 
		else if(axis == ZAXIS) 
		{
      if(planePosition == 0) 
			{
        planeDirection = POS_Z;
      } 
			else 
			{
        planeDirection = NEG_Z;
      }
    }
    timer = 0;
    looped = false;
    loading = false;
  }
	
  timer++;
  if(timer > PLANE_BOING_TIME) 
	{
    timer = 0;
    shift(planeDirection);
		
    if(planeDirection % 2 == 0) 
		{
      planePosition++;
      if (planePosition == 7) 
			{
        if(looped) 
				{
          loading = true;
        } 
				else 
				{
          planeDirection++;
          looped = true;
        }
      }
    } 
		else 
		{
      planePosition--;
      if(planePosition == 0) 
			{
        if (looped) 
				{
          loading = true;
        } 
				else 
				{
          planeDirection--;
          looped = true;
        }
      }
    }
  }
}

void sendVoxels() 
{
	uint8_t selX = 0;
	uint8_t selY = 0;
	uint8_t selZ = 0;
	uint8_t sendDirection = 0;
	bool sending = false;
	//srand((LL_RTC_DATE_Get(RTC))*(LL_RTC_TIME_Get(RTC)));//seed for random
	//srand(LL_RTC_TIME_Get(RTC));
	
  if(loading) 
	{
    clearCube();
    for(uint8_t x = 0; x < 8; x++) 
		{
      for(uint8_t z = 0; z < 8; z++) 
			{
        //setVoxel(x, random(0, 2) * 7, z);
				//setVoxel(x, rand()%2 * 7, z);
      }
    }
    loading = false;
  }
  timer++;
  if(timer > SEND_VOXELS_TIME) 
	{
    timer = 0;
    if(!sending) 
		{
      //selX = random(0, 8);
			//selX = rand()%8;
			
      //selZ = random(0, 8);
			//selZ = rand()%8;
			
      if (getVoxel(selX, 0, selZ)) 
			{
        selY = 0;
        sendDirection = POS_Y;
      } 
			else if (getVoxel(selX, 7, selZ)) 
			{
        selY = 7;
        sendDirection = NEG_Y;
      }
      sending = true;
    } 
		else 
		{
      if(sendDirection == POS_Y) 
			{
        selY++;
        setVoxel(selX, selY, selZ);
        clearVoxel(selX, selY - 1, selZ);
        if(selY == 7) 
				{
          sending = false;
        }
      } 
			else
			{
        selY--;
        setVoxel(selX, selY, selZ);
        clearVoxel(selX, selY + 1, selZ);
        if (selY == 0)
				{
          sending = false;
        }
      }
    }
  }
}

void woopWoop() 
{
	uint8_t cubeSize = 0;
	bool cubeExpanding = true;
	
  if (loading) 
	{
    clearCube();
    cubeSize = 2;
    cubeExpanding = true;
    loading = false;
  }

  timer++;
  if(timer > WOOP_WOOP_TIME)
	{
    timer = 0;
    if(cubeExpanding) 
		{
      cubeSize += 2;
      if(cubeSize == 8) 
			{
        cubeExpanding = false;
      }
    } 
		else 
		{
      cubeSize -= 2;
      if(cubeSize == 2) 
			{
        cubeExpanding = true;
      }
    }
    clearCube();
    drawCube(4 - cubeSize / 2, 4 - cubeSize / 2, 4 - cubeSize / 2, cubeSize);
  }
}

void cubeJump() 
{
	uint8_t cubeSize = 0;
	bool cubeExpanding = true;
	
	uint8_t xPos;
	uint8_t yPos;
	uint8_t zPos;
  if(loading) 
	{
    clearCube();
    //xPos = random(0, 2) * 7;
    //yPos = random(0, 2) * 7;
    //zPos = random(0, 2) * 7;
		
		//xPos = rand()%2 * 7;
    //yPos = rand()%2 * 7;
    //zPos = rand()%2 * 7;
		
		
    cubeSize = 8;
    cubeExpanding = false;
    loading = false;
  }

  timer++;
  if(timer > CUBE_JUMP_TIME) 
	{
    timer = 0;
    clearCube();
    if(xPos == 0 && yPos == 0 && zPos == 0) 
		{
      drawCube(xPos, yPos, zPos, cubeSize);
    }
		else if(xPos == 7 && yPos == 7 && zPos == 7) 
		{
      drawCube(xPos + 1 - cubeSize, yPos + 1 - cubeSize, zPos + 1 - cubeSize, cubeSize);
    } 
		else if(xPos == 7 && yPos == 0 && zPos == 0) 
		{
      drawCube(xPos + 1 - cubeSize, yPos, zPos, cubeSize);
    } 
		else if(xPos == 0 && yPos == 7 && zPos == 0) 
		{
      drawCube(xPos, yPos + 1 - cubeSize, zPos, cubeSize);
    } 
		else if(xPos == 0 && yPos == 0 && zPos == 7) 
		{
      drawCube(xPos, yPos, zPos + 1 - cubeSize, cubeSize);
    } 
		else if(xPos == 7 && yPos == 7 && zPos == 0) 
		{
      drawCube(xPos + 1 - cubeSize, yPos + 1 - cubeSize, zPos, cubeSize);
    } 
		else if(xPos == 0 && yPos == 7 && zPos == 7) 
		{
      drawCube(xPos, yPos + 1 - cubeSize, zPos + 1 - cubeSize, cubeSize);
    } 
		else if(xPos == 7 && yPos == 0 && zPos == 7) 
		{
      drawCube(xPos + 1 - cubeSize, yPos, zPos + 1 - cubeSize, cubeSize);
    }
    if(cubeExpanding) 
		{
      cubeSize++;
      if(cubeSize == 8) 
			{
        cubeExpanding = false;
        //xPos = random(0, 2) * 7;
        //yPos = random(0, 2) * 7;
        //zPos = random(0, 2) * 7;
				
				
				//xPos = rand()%2 * 7;
        //yPos = rand()%2 * 7;
        //zPos = rand()%2 * 7;
      }
    } 
		else 
		{
      cubeSize--;
      if (cubeSize == 1) 
			{
        cubeExpanding = true;
      }
    }
  }
}

void glow() 
{
	bool glowing;
	uint16_t glowCount = 0;
	
	uint8_t selX = 0;
	uint8_t selY = 0;
	uint8_t selZ = 0;
	srand((LL_RTC_DATE_Get(RTC))*(LL_RTC_TIME_Get(RTC)));//seed for random
  if(loading) 
	{
    clearCube();
    glowCount = 0;
    glowing = true;
    loading = false;
  }

  timer++;
  if(timer > GLOW_TIME) 
	{
    timer = 0;
    if(glowing) 
		{
      if(glowCount < 448) 
			{
        do 
				{
          //selX = random(0, 8);
					//selX = rand()%8;
					
          //selY = random(0, 8);
					//selY =rand()%8;''
					
          //selZ = random(0, 8);
					//selZ = rand()%8;''
					
        } while(getVoxel(selX, selY, selZ));
				
        setVoxel(selX, selY, selZ);
        glowCount++;
      } 
			else if(glowCount < 512) 
			{
        lightCube();
        glowCount++;
      } 
			else 
			{
        glowing = false;
        glowCount = 0;
      }
    } 
		else 
		{
      if(glowCount < 448) 
			{
        do 
				{
          //selX = random(0, 8);
					//selX = rand()%8;
					
          //selY = random(0, 8);
					//selY =rand()%8;''
					
          //selZ = random(0, 8);
					//selZ = rand()%8;''
        } while(!getVoxel(selX, selY, selZ));
				
        clearVoxel(selX, selY, selZ);
        glowCount++;
      } 
			else 
			{
        clearCube();
        glowing = true;
        glowCount = 0;
      }
    }
  }
}

void text(char string[], uint8_t len) 
{
	uint8_t charCounter = 0;
	uint8_t charPosition = 0;
	
  if(loading)
	{
    clearCube();
    charPosition = -1;
    charCounter = 0;
    loading = false;
  }
  timer++;
  if(timer > TEXT_TIME) 
	{
    timer = 0;

    shift(NEG_Z);
    charPosition++;

    if(charPosition == 7) 
		{
      charCounter++;
      if (charCounter > len - 1) 
			{
        charCounter = 0;
      }
      charPosition = 0;
    }

    if(charPosition == 0) 
		{
      for(uint8_t i = 0; i < 8; i++) 
			{
        cube[i][0] = characters[string[charCounter] - '0'][i];
      }
    }
  }
}

void lit() 
{
  if (loading) 
	{
    clearCube();
    for(uint8_t i=0; i<8; i++) 
		{
      for(uint8_t j=0; j<8; j++) 
			{
        cube[i][j] = 0xFF;
      }
    }
    loading = false;
  }
}

void setVoxel(uint8_t x, uint8_t y, uint8_t z) 
{
  cube[7 - y][7 - z] |= (0x01 << x);
}

void clearVoxel(uint8_t x, uint8_t y, uint8_t z) 
{
  cube[7 - y][7 - z] ^= (0x01 << x);
}

bool getVoxel(uint8_t x, uint8_t y, uint8_t z) 
{
  return (cube[7 - y][7 - z] & (0x01 << x)) == (0x01 << x);
}

void setPlane(uint8_t axis, uint8_t i) 
{
  for (uint8_t j = 0; j < 8; j++) 
	{
    for (uint8_t k = 0; k < 8; k++) 
		{
      if (axis == XAXIS) 
			{
        setVoxel(i, j, k);
      } 
			else if (axis == YAXIS) 
			{
        setVoxel(j, i, k);
      } 
			else if (axis == ZAXIS) 
			{
        setVoxel(j, k, i);
      }
    }
  }
}

void shift(uint8_t dir) 
{
  if(dir == POS_X) 
	{
    for(uint8_t y = 0; y < 8; y++) 
		{
      for(uint8_t z = 0; z < 8; z++) 
			{
        cube[y][z] = cube[y][z] << 1;
      }
    }
  } 
	else if(dir == NEG_X) 
	{
    for (uint8_t y = 0; y < 8; y++) 
		{
      for (uint8_t z = 0; z < 8; z++) 
			{
        cube[y][z] = cube[y][z] >> 1;
      }
    }
  } 
	else if(dir == POS_Y) 
	{
    for (uint8_t y = 1; y < 8; y++) 
		{
      for (uint8_t z = 0; z < 8; z++) 
			{
        cube[y - 1][z] = cube[y][z];
      }
    }
    for(uint8_t i = 0; i < 8; i++) 
		{
      cube[7][i] = 0;
    }
  } 
	else if(dir == NEG_Y) 
	{
    for(uint8_t y = 7; y > 0; y--) 
		{
      for(uint8_t z = 0; z < 8; z++) 
			{
        cube[y][z] = cube[y - 1][z];
      }
    }
    for(uint8_t i = 0; i < 8; i++) 
		{
      cube[0][i] = 0;
    }
  } 
	else if (dir == POS_Z) 
	{
    for (uint8_t y = 0; y < 8; y++) 
		{
      for (uint8_t z = 1; z < 8; z++) 
			{
        cube[y][z - 1] = cube[y][z];
      }
    }
    for (uint8_t i = 0; i < 8; i++) 
		{
      cube[i][7] = 0;
    }
  } 
	else if (dir == NEG_Z) 
	{
    for (uint8_t y = 0; y < 8; y++) 
		{
      for (uint8_t z = 7; z > 0; z--) 
			{
        cube[y][z] = cube[y][z - 1];
      }
    }
    for (uint8_t i = 0; i < 8; i++) 
		{
      cube[i][0] = 0;
    }
  }
}

void drawCube(uint8_t x, uint8_t y, uint8_t z, uint8_t s) 
{
  for(uint8_t i = 0; i < s; i++) 
	{
    setVoxel(x, y + i, z);
    setVoxel(x + i, y, z);
    setVoxel(x, y, z + i);
    setVoxel(x + s - 1, y + i, z + s - 1);
    setVoxel(x + i, y + s - 1, z + s - 1);
    setVoxel(x + s - 1, y + s - 1, z + i);
    setVoxel(x + s - 1, y + i, z);
    setVoxel(x, y + i, z + s - 1);
    setVoxel(x + i, y + s - 1, z);
    setVoxel(x + i, y, z + s - 1);
    setVoxel(x + s - 1, y, z + i);
    setVoxel(x, y + s - 1, z + i);
  }
}

void lightCube() 
{
  for (uint8_t i = 0; i < 8; i++) 
	{
    for (uint8_t j = 0; j < 8; j++) 
		{
      cube[i][j] = 0xFF;
    }
  }
}

void clearCube() 
{
  for (uint8_t i = 0; i < 8; i++) 
	{
    for (uint8_t j = 0; j < 8; j++) 
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
}void SystemClock_Config(void)
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