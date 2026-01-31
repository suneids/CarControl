#include "myhal.h"
#include "button.h"

typedef struct {
 unsigned char deg;
 signed char speed;
 unsigned char xor;
}Packet;

typedef struct {
 uint32_t deg[2];
 uint32_t speed;
}PacketNormalized;

typedef enum{
	MOBILE_APP = 0,
	EMPOWER_GLOVE,
	MODES_AMOUNT
}Modes_t;

PacketNormalized PreparePacket(Packet p);
PacketNormalized FixTurn(PacketNormalized p);
void Normalize(PacketNormalized *p);


Modes_t mode = 0;
Pin_t rightWheel = {GPIOA, 0}, leftWheel = {GPIOA, 1},
	  engine1 = {GPIOA, 2}, engine2 = {GPIOA, 3},
	  mode_btn = {GPIOB, 0},
	  SUtx = {GPIOB, 10}, SUrx = {GPIOB, 11},
	  led = {GPIOC, 13};


const uint32_t PWMMax = 1999,
   EnginePWMMax = PWMMax,
   servoMax = PWMMax/10,
   servoMin = PWMMax/20,
   servoRange = servoMax - servoMin;

PacketNormalized np;
uint8_t forward = 1, packet_start = 0, nextByte = 0;
uint32_t lWheelforward = 78, rWheelforward = 102;
uint8_t differenceR = 12, differenceL = 12;
char currentPacket[4] = { 0 };

Packet packet = {0, 0, 0};
uint32_t last_check_time = 0;

int main(void) {
	RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;

	usartInit(USART1, 9600);
	softUartInit(SUrx, SUtx, 9600);
	timerInit(TIM2, 71, PWMMax, 0);
	//Настройка портов для ШИМ
	pinMode(rightWheel, GPIO_MODE_OUTPUT_10MHz, GPIO_CNF_PUSH_PULL_ALT, 0);
	pinMode(leftWheel, GPIO_MODE_OUTPUT_10MHz, GPIO_CNF_PUSH_PULL_ALT, 0);
	pinMode(engine1, GPIO_MODE_OUTPUT_10MHz, GPIO_CNF_PUSH_PULL_ALT, 0);
	pinMode(engine2, GPIO_MODE_OUTPUT_10MHz, GPIO_CNF_PUSH_PULL_ALT, 0);

	pinMode(led, GPIO_MODE_OUTPUT_2MHz, GPIO_CNF_ANALOG, 1);

	pwmInit(rightWheel);
	pwmInit(leftWheel);
	pwmInit(engine1);
	pwmInit(engine2);

	pwmWrite(rightWheel, rWheelforward);
	pwmWrite(leftWheel, lWheelforward);
	pwmWrite(engine1, 0);
	pwmWrite(engine2, 0);

	pinMode(mode_btn, GPIO_MODE_INPUT, GPIO_CNF_INPUT_PU_PD, 1);
	sysTickInit();
    while(1){
    	uint8_t packet_taked = 0;
    	if(checkButtonPress(mode_btn) == BTN_SHORT_CLICK){
    		mode = (mode + 1) % MODES_AMOUNT;
    	}

    	switch(mode){
    		case MOBILE_APP:{
    			if(usartAvailable(USART1) >= 4){
    				pinToggle(led);
					uint8_t start = usartReadByte(USART1);
					if(start != 0xFE) continue; // Пропускаем каждый байт, что не стартовый
					// Читаем остальные 3 байта
					packet.deg = usartReadByte(USART1);
					packet.speed = (int8_t)usartReadByte(USART1);
					packet.xor = usartReadByte(USART1);
					packet_taked = 1;
				}
    			break;
    		}
    		case EMPOWER_GLOVE:{

    			if(softUartAvailable() >= 4){
    				pinToggle(led);
					uint8_t start = softUartReadByte();
					if(start != 0xFE) continue; // Пропускаем каждый байт, что не стартовый
					// Читаем остальные 3 байта
					packet.deg = softUartReadByte();
					packet.speed = (int8_t)softUartReadByte();
					packet.xor = softUartReadByte();
					packet_taked = 1;

				}
    			break;
    		}
    		default:{
    			break;
    		}
    	}

    	if(packet_taked){
			// Проверка XOR
			if(((uint8_t)packet.deg ^ (uint8_t)packet.speed) != packet.xor) continue;
			 if(packet.speed < 0){
				 packet.speed = -packet.speed;
				 forward = 0;
			 }
			 else{
				 forward = 1;
			 }
			 np = PreparePacket(packet);
			 np = FixTurn(np);
			 Normalize(&np);

			 // PWM серв
			 pwmWrite(rightWheel, np.deg[0]);
			 pwmWrite(leftWheel, np.deg[1]);
			 // PWM мотор
			 if(forward){
				 pwmWrite(engine1, np.speed);
				 pwmWrite(engine2, 0);
			 }
			 else {
				 pwmWrite(engine1, 0);
				 pwmWrite(engine2, np.speed);
			 }
		}
	}
}


PacketNormalized PreparePacket(Packet p){
	 PacketNormalized result;
	 result.speed = p.speed;
	 result.deg[0] = p.deg;
	 result.deg[1] = p.deg;
	 return result;
}


PacketNormalized FixTurn(PacketNormalized p){
	PacketNormalized result = p;

	uint32_t degRight = p.deg[1] + differenceR;
	uint32_t degLeft = p.deg[0] - differenceL;
	if(degRight > rWheelforward){
		degRight+=20;
		if(degRight > 180) degRight = 180;
	}
	if(degLeft < lWheelforward){
		degLeft-=20;
		if(degLeft < 0) degLeft = 0;
	}

	result.deg[0] = degRight;
	result.deg[1] = degLeft;
	return result;
}


void Normalize(PacketNormalized *p){
	p->speed = (EnginePWMMax  * p->speed) / 100;
	p->deg[0] = servoMin + servoRange * p->deg[0] / 180;
	p->deg[1] = servoMin + servoRange * p->deg[1] / 180;
}
