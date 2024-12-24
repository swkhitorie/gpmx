
#ifndef __LLD_H7_GLOBALPIN_H_
#define __LLD_H7_GLOBALPIN_H_

#include <stdint.h>
#include <stdbool.h>
#include "stm32h7xx.h"

#define SECTIONS_H7_RAMD1 ".RAM_D1"

typedef enum __lld_rwway {
    RWPOLL,
    RWIT,
    RWDMA
} lld_rwway;

/**
 * @brief the abstraction of peripherals's pin resource
*/
struct pin_node{
	GPIO_TypeDef *gpio_port;
	uint8_t pin_num;
	uint32_t alternate;
};

#define INDEX(a)  a
#define PINNODE(a) (a)
//#define PINNODE(a) reinterpret_cast<a>

/**
 * @brief 
 *  =========================================================================
 * ||																		||
 * ||																		||
 * ||			all uart device pin resource in  STM32H743IIT6  			||
 * ||																		||
 * ||																		||
 *  =========================================================================
*/
#define		UART_PIN_TX	 		0
#define		UART_PIN_RX  		1
#define		UART_PIN_TYPE_NUM	2
static const uint8_t uart_max_selection_number[8] = { 3, 2, 3, 7, 3, 2, 4, 2 };

static const struct pin_node usart1_pinctrl[2][3] = {
/*--------- TX ---------*/
{ {GPIOA, 9, GPIO_AF7_USART1}, {GPIOB, 6, GPIO_AF7_USART1}, {GPIOB, 14, GPIO_AF4_USART1} },
/*--------- RX ---------*/
{ {GPIOA, 10, GPIO_AF7_USART1}, {GPIOB, 7, GPIO_AF7_USART1}, {GPIOB, 15, GPIO_AF4_USART1} },
};

static const struct pin_node usart2_pinctrl[2][2] = {
/*--------- TX ---------*/
{ {GPIOA, 2, GPIO_AF7_USART2}, {GPIOD, 5, GPIO_AF7_USART2} },
/*--------- RX ---------*/
{ {GPIOA, 3, GPIO_AF7_USART2}, {GPIOD, 6, GPIO_AF7_USART2} },
};

static const struct pin_node usart3_pinctrl[2][3] = {
/*--------- TX ---------*/
{ {GPIOB, 10, GPIO_AF7_USART3}, {GPIOC, 10, GPIO_AF7_USART3}, {GPIOD, 8, GPIO_AF7_USART3} },
/*--------- RX ---------*/
{ {GPIOB, 11, GPIO_AF7_USART3}, {GPIOC, 11, GPIO_AF7_USART3}, {GPIOD, 9, GPIO_AF7_USART3} },
};


static const struct pin_node uart4_pinctrl[2][7] = {
/*--------- TX ---------*/
{ {GPIOA, 12, GPIO_AF6_UART4}, {GPIOA, 0, GPIO_AF8_UART4}, {GPIOB, 9, GPIO_AF8_UART4}, {GPIOC, 10, GPIO_AF8_UART4},
	{GPIOD, 1, GPIO_AF8_UART4}, {GPIOH, 13, GPIO_AF8_UART4}, {0, 0, 0} },
/*--------- RX ---------*/
{ {GPIOA, 11, GPIO_AF6_UART4}, {GPIOA, 1, GPIO_AF8_UART4}, {GPIOB, 8, GPIO_AF8_UART4}, {GPIOC, 11, GPIO_AF8_UART4},
	{GPIOD, 0, GPIO_AF8_UART4}, {GPIOH, 14, GPIO_AF8_UART4}, {GPIOI, 9, GPIO_AF8_UART4} },
};

static const struct pin_node uart5_pinctrl[2][3] = {
/*--------- TX ---------*/
{ {GPIOC, 12, GPIO_AF8_UART5}, {GPIOB, 6, GPIO_AF14_UART5}, {GPIOB, 13, GPIO_AF14_UART5} },
/*--------- RX ---------*/
{ {GPIOD, 2, GPIO_AF8_UART5}, {GPIOB, 5, GPIO_AF14_UART5}, {GPIOB, 12, GPIO_AF14_UART5} },
};

static const struct pin_node usart6_pinctrl[2][2] = {
/*--------- TX ---------*/
{ {GPIOC, 6, GPIO_AF7_USART6}, {GPIOG, 14, GPIO_AF7_USART6} },
/*--------- RX ---------*/
{ {GPIOC, 7, GPIO_AF7_USART6}, {GPIOG, 9, GPIO_AF7_USART6} },
};

static const struct pin_node uart7_pinctrl[2][4] = {
/*--------- TX ---------*/
{ {GPIOE, 8, GPIO_AF7_UART7}, {GPIOF, 7, GPIO_AF7_UART7}, {GPIOA, 15, GPIO_AF11_UART7}, {GPIOB, 4, GPIO_AF11_UART7} },
/*--------- RX ---------*/
{ {GPIOE, 7, GPIO_AF7_UART7}, {GPIOF, 6, GPIO_AF7_UART7}, {GPIOA, 8, GPIO_AF11_UART7}, {GPIOB, 3, GPIO_AF11_UART7} },
};

static const struct pin_node uart8_pinctrl[2][2] = {
/*--------- TX ---------*/
{ {GPIOE, 1, GPIO_AF8_UART8}, {GPIOJ, 8, GPIO_AF8_UART8} },
/*--------- RX ---------*/
{ {GPIOE, 0, GPIO_AF8_UART8}, {GPIOJ, 9, GPIO_AF8_UART8} },
};

#define USART_PINCTRL_SOURCE(NUM, TYPE, SELECT)   \
		(TYPE < UART_PIN_TYPE_NUM && SELECT <= uart_max_selection_number[NUM - 1]) ?   \
				&usart## NUM ##_pinctrl[INDEX(TYPE)][INDEX(SELECT) - 1] : NULL

#define UART_PINCTRL_SOURCE(NUM, TYPE, SELECT)   \
		(TYPE < UART_PIN_TYPE_NUM && SELECT <= uart_max_selection_number[NUM - 1]) ?   \
				&uart## NUM ##_pinctrl[INDEX(TYPE)][INDEX(SELECT) - 1] : NULL

/**
 * @brief 
 *  =========================================================================
 * ||																		||
 * ||																		||
 * ||			all timer device pin resource in  STM32H743IIT6  			||
 * ||																		||
 * ||																		||
 *  =========================================================================
*/
#define		TIM_PIN_CHANNEL_1	 	0
#define		TIM_PIN_CHANNEL_2  		1
#define		TIM_PIN_CHANNEL_3	 	2
#define		TIM_PIN_CHANNEL_4  		3
#define		TIM_PIN_ETR  			4				
#define		TIM_PIN_TYPE_NUM		5
static const uint8_t tim_max_selection_number[8] = { 3, 3, 3, 2, 2, 0, 0, 4 };



static const struct pin_node timer1_pinctrl[5][3] = {
/*--------- CHANNEL_1 ---------*/
{ {GPIOA, 8, GPIO_AF1_TIM1}, {GPIOE, 9, GPIO_AF1_TIM1}, {GPIOK, 1, GPIO_AF1_TIM1} },

/*--------- CHANNEL_2 ---------*/
{ {GPIOA, 9, GPIO_AF1_TIM1}, {GPIOE, 11, GPIO_AF1_TIM1}, {GPIOJ, 11, GPIO_AF1_TIM1} },

/*--------- CHANNEL_3 ---------*/
{ {GPIOA, 10, GPIO_AF1_TIM1}, {GPIOE, 13, GPIO_AF1_TIM1}, {GPIOJ, 9, GPIO_AF1_TIM1} },

/*--------- CHANNEL_4 ---------*/
{ {GPIOA, 11, GPIO_AF1_TIM1}, {GPIOE, 14, GPIO_AF1_TIM1}, {0, 0, 0} },

/*--------- ETR ---------*/
{ {GPIOA, 12, GPIO_AF1_TIM1}, {GPIOE, 7, GPIO_AF1_TIM1}, {GPIOG, 5, GPIO_AF1_TIM1} },

};

static const struct pin_node timer2_pinctrl[5][3] = {
/*--------- CHANNEL_1 ---------*/
{ {GPIOA, 0, GPIO_AF1_TIM2}, {GPIOA, 5, GPIO_AF1_TIM2}, {GPIOA, 15, GPIO_AF1_TIM2} },

/*--------- CHANNEL_2 ---------*/
{ {GPIOA, 1, GPIO_AF1_TIM2}, {GPIOB, 3, GPIO_AF1_TIM2}, {0, 0, 0} },

/*--------- CHANNEL_3 ---------*/
{ {GPIOA, 2, GPIO_AF1_TIM2}, {GPIOB, 10, GPIO_AF1_TIM2}, {0, 0, 0} },

/*--------- CHANNEL_4 ---------*/
{ {GPIOA, 3, GPIO_AF1_TIM2}, {GPIOB, 11, GPIO_AF1_TIM2}, {0, 0, 0} },

/*--------- ETR ---------*/
{ {GPIOA, 0, GPIO_AF1_TIM2}, {GPIOA, 5, GPIO_AF1_TIM2}, {GPIOA, 15, GPIO_AF1_TIM2} },

};

static const struct pin_node timer3_pinctrl[5][3] = {
/*--------- CHANNEL_1 ---------*/
{ {GPIOA, 6, GPIO_AF2_TIM3}, {GPIOB, 4, GPIO_AF2_TIM3}, {GPIOC, 6, GPIO_AF2_TIM3} },

/*--------- CHANNEL_2 ---------*/
{ {GPIOA, 7, GPIO_AF2_TIM3}, {GPIOB, 5, GPIO_AF2_TIM3}, {GPIOC, 7, GPIO_AF2_TIM3} },

/*--------- CHANNEL_3 ---------*/
{ {GPIOB, 0, GPIO_AF2_TIM3}, {GPIOC, 8, GPIO_AF2_TIM3}, {0, 0, 0} },

/*--------- CHANNEL_4 ---------*/
{ {GPIOB, 1, GPIO_AF2_TIM3}, {GPIOC, 9, GPIO_AF2_TIM3}, {0, 0, 0} },

/*--------- ETR ---------*/
{ {GPIOD, 2, GPIO_AF2_TIM3}, {0, 0, 0}, {0, 0, 0}},

};

static const struct pin_node timer4_pinctrl[5][2] = {
/*--------- CHANNEL_1 ---------*/
{ {GPIOB, 6, GPIO_AF2_TIM4}, {GPIOD, 12, GPIO_AF2_TIM4} },

/*--------- CHANNEL_2 ---------*/
{ {GPIOB, 7, GPIO_AF2_TIM4}, {GPIOD, 13, GPIO_AF2_TIM4} },

/*--------- CHANNEL_3 ---------*/
{ {GPIOB, 8, GPIO_AF2_TIM4}, {GPIOD, 14, GPIO_AF2_TIM4}},

/*--------- CHANNEL_4 ---------*/
{ {GPIOB, 9, GPIO_AF2_TIM4}, {GPIOD, 15, GPIO_AF2_TIM4} },

/*--------- ETR ---------*/
{ {GPIOE, 0, GPIO_AF2_TIM4}, {0, 0, 0} },

};

static const struct pin_node timer5_pinctrl[5][2] = {
/*--------- CHANNEL_1 ---------*/
{ {GPIOA, 0, GPIO_AF2_TIM5}, {GPIOH, 10, GPIO_AF2_TIM5} },

/*--------- CHANNEL_2 ---------*/
{ {GPIOA, 1, GPIO_AF2_TIM5}, {GPIOH, 11, GPIO_AF2_TIM5} },

/*--------- CHANNEL_3 ---------*/
{ {GPIOA, 2, GPIO_AF2_TIM5}, {GPIOH, 12, GPIO_AF2_TIM5}},

/*--------- CHANNEL_4 ---------*/
{ {GPIOA, 3, GPIO_AF2_TIM5}, {GPIOI, 0, GPIO_AF2_TIM5} },

/*--------- ETR ---------*/
{ {GPIOA, 4, GPIO_AF2_TIM5}, {GPIOH, 8, GPIO_AF2_TIM5} },

};

static const struct pin_node timer8_pinctrl[5][4] = {
/*--------- CHANNEL_1 ---------*/
{ {GPIOC, 6, GPIO_AF3_TIM8}, {GPIOI, 5, GPIO_AF3_TIM8}, {GPIOJ, 8, GPIO_AF3_TIM8}, {0, 0, 0} },

/*--------- CHANNEL_2 ---------*/
{ {GPIOC, 7, GPIO_AF3_TIM8}, {GPIOI, 6, GPIO_AF3_TIM8}, {GPIOJ, 6, GPIO_AF3_TIM8}, {GPIOJ, 10, GPIO_AF3_TIM8} },

/*--------- CHANNEL_3 ---------*/
{ {GPIOC, 8, GPIO_AF3_TIM8}, {GPIOI, 7, GPIO_AF3_TIM8}, {GPIOK, 0, GPIO_AF3_TIM8}, {0, 0, 0} },

/*--------- CHANNEL_4 ---------*/
{ {GPIOC, 9, GPIO_AF3_TIM8}, {GPIOI, 2, GPIO_AF3_TIM8}, {0, 0, 0}, {0, 0, 0} },

/*--------- ETR ---------*/
{ {GPIOA, 0, GPIO_AF3_TIM8}, {GPIOG, 8, GPIO_AF3_TIM8}, {GPIOI, 3, GPIO_AF3_TIM8}, {0, 0, 0} },

};

#define TIMER_PINCTRL_SOURCE(NUM, TYPE, SELECT)   \
		(TYPE < TIM_PIN_TYPE_NUM && SELECT <= tim_max_selection_number[NUM - 1]) ?   \
				&timer## NUM ##_pinctrl[INDEX(TYPE)][INDEX(SELECT) - 1] : NULL

/**
 *  =========================================================================
 * ||																		||
 * ||																		||
 * ||			another timer resource integration method in H743			||
 * ||																		||
 * ||																		||
 *  =========================================================================
*/
#define		TIMER_IS_FLASH_LARGE	0
#if TIMER_IS_FLASH_LARGE 
	static const struct pin_node all_timer_pinctrl[8][5][3] = {
	/*-------------------------------- TIMER 1 --------------------------------*/
	{
		/*---------------------- CHANNEL 1 ----------------------*/
	{{GPIOE,9,GPIO_AF1_TIM1}, 	{GPIOK,1,GPIO_AF1_TIM1}, 	{GPIOA,8,GPIO_AF1_TIM1}},
		/*---------------------- CHANNEL 2 ----------------------*/
	{{GPIOE,11,GPIO_AF1_TIM1}, 	{GPIOJ,11,GPIO_AF1_TIM1}, 	{GPIOA,9,GPIO_AF1_TIM1}},
		/*---------------------- CHANNEL 3 ----------------------*/
	{{GPIOE,13,GPIO_AF1_TIM1},	{GPIOJ,9,GPIO_AF1_TIM1},	{GPIOA,10,GPIO_AF1_TIM1}},
		/*---------------------- CHANNEL 4 ----------------------*/
	{{GPIOE,14,GPIO_AF1_TIM1}, 	{0,0,0},     				{GPIOA,11,GPIO_AF1_TIM1}},
		/*------------------------ ETR ------------------------*/
	{{GPIOE,7,GPIO_AF1_TIM1}, 	{GPIOG,5,GPIO_AF1_TIM1}, 	{GPIOA,12,GPIO_AF1_TIM1}}
	},

	/*-------------------------------- TIMER 2 --------------------------------*/
	{
		/*---------------------- CHANNEL 1 ----------------------*/
	{{GPIOA,0,GPIO_AF1_TIM2}, 	{GPIOA,5,GPIO_AF1_TIM2}, 	{GPIOA,15,GPIO_AF1_TIM2}},
		/*---------------------- CHANNEL 2 ----------------------*/
	{{GPIOA,1,GPIO_AF1_TIM2}, 	{GPIOB,3,GPIO_AF1_TIM2}, 	{0,0,0}},
		/*---------------------- CHANNEL 3 ----------------------*/
	{{GPIOA,2,GPIO_AF1_TIM2},	{GPIOB,10,GPIO_AF1_TIM2},	{0,0,0}},
		/*---------------------- CHANNEL 4 ----------------------*/
	{{GPIOA,3,GPIO_AF1_TIM2}, 	{GPIOB,11,GPIO_AF1_TIM2},   {0,0,0}},
		/*------------------------ ETR ------------------------*/
	{{GPIOA,0,GPIO_AF1_TIM2}, 	{GPIOA,5,GPIO_AF1_TIM2}, 	{GPIOA,15,GPIO_AF1_TIM2}}
	},

	/*-------------------------------- TIMER 3 --------------------------------*/
	{
		/*---------------------- CHANNEL 1 ----------------------*/
	{{GPIOA,6,GPIO_AF2_TIM3}, 	{GPIOC,6,GPIO_AF2_TIM3}, 	{GPIOB,4,GPIO_AF2_TIM3}},
		/*---------------------- CHANNEL 2 ----------------------*/
	{{GPIOA,7,GPIO_AF2_TIM3}, 	{GPIOC,7,GPIO_AF2_TIM3}, 	{GPIOB,5,GPIO_AF2_TIM3}},
		/*---------------------- CHANNEL 3 ----------------------*/
	{{GPIOB,0,GPIO_AF2_TIM3},	{GPIOC,8,GPIO_AF2_TIM3},	{0,0,0}},
		/*---------------------- CHANNEL 4 ----------------------*/
	{{GPIOB,1,GPIO_AF2_TIM3},	{GPIOC,9,GPIO_AF2_TIM3},	{0,0,0}},
		/*------------------------ ETR ------------------------*/
	{{GPIOD,2,GPIO_AF2_TIM3}, 	{0,0,0}, 					{0,0,0}}
	},

	/*-------------------------------- TIMER 4 --------------------------------*/
	{
		/*---------------------- CHANNEL 1 ----------------------*/
	{{GPIOD,12,GPIO_AF2_TIM4}, 	{GPIOB,6,GPIO_AF2_TIM4}, 	{0,0,0}},
		/*---------------------- CHANNEL 2 ----------------------*/
	{{GPIOD,13,GPIO_AF2_TIM4}, 	{GPIOB,7,GPIO_AF2_TIM4}, 	{0,0,0}},
		/*---------------------- CHANNEL 3 ----------------------*/
	{{GPIOD,14,GPIO_AF2_TIM4}, 	{GPIOB,8,GPIO_AF2_TIM4}, 	{0,0,0}},
		/*---------------------- CHANNEL 4 ----------------------*/
	{{GPIOD,15,GPIO_AF2_TIM4}, 	{GPIOB,9,GPIO_AF2_TIM4}, 	{0,0,0}},
		/*------------------------ ETR ------------------------*/
	{{GPIOE,0,GPIO_AF2_TIM4}, 	{0,0,0}, 					{0,0,0}}
	},

	/*-------------------------------- TIMER 5 --------------------------------*/
	{
		/*---------------------- CHANNEL 1 ----------------------*/
	{{GPIOA,0,GPIO_AF2_TIM5}, 	{GPIOH,10,GPIO_AF2_TIM5}, 	{0,0,0}},
		/*---------------------- CHANNEL 2 ----------------------*/
	{{GPIOA,1,GPIO_AF2_TIM5}, 	{GPIOH,11,GPIO_AF2_TIM5}, 	{0,0,0}},
		/*---------------------- CHANNEL 3 ----------------------*/
	{{GPIOA,2,GPIO_AF2_TIM5}, 	{GPIOH,12,GPIO_AF2_TIM5}, 	{0,0,0}},
		/*---------------------- CHANNEL 4 ----------------------*/
	{{GPIOA,3,GPIO_AF2_TIM5}, 	{GPIOI,0,GPIO_AF2_TIM5}, 	{0,0,0}},
		/*------------------------ ETR ------------------------*/
	{{GPIOA,4,GPIO_AF2_TIM5}, 	{GPIOH,8,GPIO_AF2_TIM5}, 	{0,0,0}}
	},

	/*-------------------------------- TIMER 6 and 7 --------------------------------*/
	{
	{{0,0,0}, 	{0,0,0}, 	{0,0,0}},
	{{0,0,0}, 	{0,0,0}, 	{0,0,0}},
	{{0,0,0}, 	{0,0,0}, 	{0,0,0}},
	{{0,0,0}, 	{0,0,0}, 	{0,0,0}},
	{{0,0,0}, 	{0,0,0}, 	{0,0,0}},
	},
	{
	{{0,0,0}, 	{0,0,0}, 	{0,0,0}},
	{{0,0,0}, 	{0,0,0}, 	{0,0,0}},
	{{0,0,0}, 	{0,0,0}, 	{0,0,0}},
	{{0,0,0}, 	{0,0,0}, 	{0,0,0}},
	{{0,0,0}, 	{0,0,0}, 	{0,0,0}},
	},

	/*-------------------------------- TIMER 8 --------------------------------*/
	{
		/*---------------------- CHANNEL 1 ----------------------*/
	{{GPIOJ,8,GPIO_AF3_TIM8}, 	{GPIOC,6,GPIO_AF3_TIM8}, 	{GPIOI,5,GPIO_AF3_TIM8}},
		/*---------------------- CHANNEL 2 ----------------------*/
	{{GPIOJ,6,GPIO_AF3_TIM8}, 	{GPIOC,7,GPIO_AF3_TIM8}, 	{GPIOI,6,GPIO_AF3_TIM8}},
		/*---------------------- CHANNEL 3 ----------------------*/
	{{GPIOK,0,GPIO_AF3_TIM8}, 	{GPIOC,8,GPIO_AF3_TIM8}, 	{GPIOI,7,GPIO_AF3_TIM8}},
		/*---------------------- CHANNEL 4 ----------------------*/
	{{0,0,0}, 					{GPIOC,9,GPIO_AF3_TIM8}, 	{GPIOI,2,GPIO_AF3_TIM8}},
		/*------------------------ ETR ------------------------*/
	{{GPIOA,0,GPIO_AF3_TIM8}, 	{GPIOG,8,GPIO_AF3_TIM8}, 	{GPIOI,3,GPIO_AF3_TIM8}}
	},

	};

	#define TIMER_GLOBAL_PINCTRL_SOURCE(TIMER_NUM, TYPE, SELECT)	all_timer_pinctrl[TIMER_NUM - 1][TYPE][SELECT - 1]
	
#else
	static const GPIO_TypeDef *all_timer_port[8][5][3] = 
	{
	/*			CH1						CH2						CH3						CH4						ETR				*/
	/*		s1		s2		s3		s1		s2		s3		s1		s2		s3		s1		s2		s3		s1		s2		s3 	*/
		{{GPIOE, GPIOK, GPIOA},  {GPIOE, GPIOJ, GPIOA},  {GPIOE, GPIOJ, GPIOA},  {GPIOE, 0,     GPIOA},  {GPIOE, GPIOG, GPIOA}},	/* TIM1 */
		{{GPIOA, GPIOA, GPIOA},  {GPIOA, GPIOB, 0    },  {GPIOA, GPIOB, 0    },  {GPIOA, GPIOB, 0    },  {GPIOA, GPIOA, GPIOA}},	/* TIM2 */
		{{GPIOA, GPIOC, GPIOB},  {GPIOA, GPIOC, GPIOB},  {GPIOB, GPIOC, 0    },  {GPIOB, GPIOC, 0    },  {GPIOD, 0,     0    }},	/* TIM3 */
		{{GPIOD, GPIOB, 0    },  {GPIOD, GPIOB, 0    },  {GPIOD, GPIOB, 0    },  {GPIOD, GPIOB, 0    },  {GPIOE, 0,     0    }},	/* TIM4 */
		{{GPIOA, GPIOH, 0    },  {GPIOA, GPIOH, 0    },  {GPIOA, GPIOH, 0    },  {GPIOA, GPIOI, 0    },  {GPIOA, GPIOH, 0    }},	/* TIM5 */
		{{0,     0,     0    },  {0,     0,     0    },  {0,     0,     0    },  {0,     0,     0    },  {0,     0,     0    }},	/* TIM6 */
		{{0,     0,     0    },  {0,     0,     0    },  {0,     0,     0    },  {0,     0,     0    },  {0,     0,     0    }},	/* TIM7 */
		
		{{GPIOJ, GPIOC, GPIOI},  {GPIOJ, GPIOC, GPIOI},  {GPIOK, GPIOC, GPIOI},  {0,     GPIOC, GPIOI},  {GPIOA, GPIOG, GPIOI}}		/* TIM8 */
		
	};

	static const uint8_t all_timer_pinnum[8][5][3] = 
	{
	/*			CH1						CH2						CH3						CH4						ETR				*/
	/*	s1		s2		s3		s1		s2		s3		s1		s2		s3		s1		s2		s3		s1		s2		s3		*/
		{{9,     1,     8    },  {11,    11,    9    },  {13,    9,     10   },  {14,    0,     11   },  {7,     5,     12    }},	/* TIM1 */
		{{0,     5,     15   },  {1 ,    3,     0    },  {2,     10,    0    },  {3,     11,    0    },  {0,     5,     15    }},	/* TIM2 */
		{{6,     6,     4    },  {7 ,    7,     5    },  {0,     8,     0    },  {1,     9,     0    },  {2,     0,     0     }},	/* TIM3 */
		{{12,    6,     0    },  {13,    7,     0    },  {14,    8,     0    },  {15,    9,     0    },  {0,     0,     0     }},	/* TIM4 */
		{{0,     10,    0    },  {1,     11,    0    },  {2,     12,    0    },  {3,     0,     0    },  {4,     8,     0     }},	/* TIM5 */
		{{0,     0,     0    },  {0,     0,     0    },  {0,     0,     0    },  {0,     0,     0    },  {0,     0,     0     }},	/* TIM6 */
		{{0,     0,     0    },  {0,     0,     0    },  {0,     0,     0    },  {0,     0,     0    },  {0,     0,     0     }},	/* TIM7 */
		{{8,     6,     5    },  {6,     7,     6    },  {0,     8,     7    },  {0,     9,     2    },  {0,     8,     3     }}	/* TIM8 */
		
	};

	static const uint8_t all_timer_pin_alternate[8] = {GPIO_AF1_TIM1, GPIO_AF1_TIM2, GPIO_AF2_TIM3, GPIO_AF2_TIM4, GPIO_AF2_TIM5, 0, 0, GPIO_AF3_TIM8};
	
	#define TIMER_GLOBAL_PORT(TIMER_NUM, TYPE, SELECT) 	all_timer_port[TIMER_NUM - 1][TYPE][SELECT - 1]
	#define TIMER_GLOBAL_PINNUM(TIMER_NUM, TYPE, SELECT) 	all_timer_pinnum[TIMER_NUM - 1][TYPE][SELECT - 1]
	#define TIMER_GLOBAL_ALTERNATE(TIMER_NUM) 	all_timer_pin_alternate[TIMER_NUM - 1]
#endif


/**
 * @brief 
 *  =========================================================================
 * ||																		||
 * ||																		||
 * ||			all spi device pin resource in  STM32H743IIT6  				||
 * ||																		||
 * ||																		||
 *  =========================================================================
*/
#define		SPI_PIN_NSS	 			0
#define		SPI_PIN_SCK		  		1
#define		SPI_PIN_MISO	 		2
#define		SPI_PIN_MOSI	  		3				
#define		SPI_PIN_TYPE_NUM		4
static const uint8_t spi_max_selection_number[6] = { 3, 6, 4, 2, 3, 3 };


static const struct pin_node spi1_pinctrl[4][3] = {
/*--------- NSS ---------*/
{ {GPIOA, 4, GPIO_AF5_SPI1}, {GPIOA, 15, GPIO_AF5_SPI1}, {GPIOG, 10, GPIO_AF5_SPI1} },

/*--------- SCK ---------*/
{ {GPIOA, 5, GPIO_AF5_SPI1}, {GPIOB, 3, GPIO_AF5_SPI1}, {GPIOG, 11, GPIO_AF5_SPI1} },

/*--------- MISO ---------*/
{ {GPIOA, 6, GPIO_AF5_SPI1}, {GPIOB, 4, GPIO_AF5_SPI1}, {GPIOG, 9, GPIO_AF5_SPI1} },

/*--------- MOSI ---------*/
{ {GPIOA, 7, GPIO_AF5_SPI1}, {GPIOB, 5, GPIO_AF5_SPI1}, {GPIOD, 7, GPIO_AF5_SPI1} },

};

static const struct pin_node spi2_pinctrl[4][6] = {
/*--------- NSS ---------*/
{ {GPIOA, 11, GPIO_AF5_SPI2}, {GPIOB, 9, GPIO_AF5_SPI2}, {GPIOB, 12, GPIO_AF5_SPI2}, {GPIOI, 0, GPIO_AF5_SPI2},
  {GPIOB, 4, GPIO_AF7_SPI2}, {0, 0, 0}
},

/*--------- SCK ---------*/
{ {GPIOA, 9, GPIO_AF5_SPI2}, {GPIOA, 12, GPIO_AF5_SPI2}, {GPIOB, 10, GPIO_AF5_SPI2}, {GPIOB, 13, GPIO_AF5_SPI2},
  {GPIOI, 1, GPIO_AF5_SPI2}, {GPIOD, 3, GPIO_AF5_SPI2}
},

/*--------- MISO ---------*/
{ {GPIOC, 2, GPIO_AF5_SPI2}, {GPIOB, 14, GPIO_AF5_SPI2}, {GPIOI, 2, GPIO_AF5_SPI2}, {0, 0, 0},
  {0, 0, 0}, {0, 0, 0}
},

/*--------- MOSI ---------*/
{ {GPIOC, 1, GPIO_AF5_SPI2}, {GPIOC, 3, GPIO_AF5_SPI2}, {GPIOB, 15, GPIO_AF5_SPI2}, {GPIOI, 3, GPIO_AF5_SPI2},
  {0, 0, 0}, {0, 0, 0}
},

};


static const struct pin_node spi3_pinctrl[4][4] = {
/*--------- NSS ---------*/
{ {GPIOA, 4, GPIO_AF6_SPI3}, {GPIOA, 15, GPIO_AF6_SPI3}, {0, 0, 0}, {0, 0, 0} },

/*--------- SCK ---------*/
{ {GPIOC, 10, GPIO_AF6_SPI3}, {GPIOB, 3, GPIO_AF6_SPI3}, {0, 0, 0}, {0, 0, 0} },

/*--------- MISO ---------*/
{ {GPIOC, 11, GPIO_AF6_SPI3}, {GPIOB, 4, GPIO_AF6_SPI3}, {0, 0, 0}, {0, 0, 0} },

/*--------- MOSI ---------*/
{ {GPIOD, 6, GPIO_AF5_SPI3}, {GPIOC, 12, GPIO_AF6_SPI3}, {GPIOB, 2, GPIO_AF7_SPI3}, {GPIOB, 5, GPIO_AF7_SPI3} },

};

static const struct pin_node spi4_pinctrl[4][2] = {
/*--------- NSS ---------*/
{ {GPIOE, 4, GPIO_AF5_SPI4}, {GPIOE, 11, GPIO_AF5_SPI4} },

/*--------- SCK ---------*/
{ {GPIOE, 2, GPIO_AF5_SPI4}, {GPIOE, 12, GPIO_AF5_SPI4} },

/*--------- MISO ---------*/
{ {GPIOE, 5, GPIO_AF5_SPI4}, {GPIOE, 13, GPIO_AF5_SPI4} },

/*--------- MOSI ---------*/
{ {GPIOE, 6, GPIO_AF5_SPI4}, {GPIOE, 14, GPIO_AF5_SPI4} },

};

static const struct pin_node spi5_pinctrl[4][3] = {
/*--------- NSS ---------*/
{ {GPIOF, 6, GPIO_AF5_SPI5}, {GPIOH, 5, GPIO_AF5_SPI5}, {GPIOK, 1, GPIO_AF5_SPI5} },

/*--------- SCK ---------*/
{ {GPIOF, 7, GPIO_AF5_SPI5}, {GPIOH, 6, GPIO_AF5_SPI5}, {GPIOK, 0, GPIO_AF5_SPI5} },

/*--------- MISO ---------*/
{ {GPIOF, 8, GPIO_AF5_SPI5}, {GPIOH, 7, GPIO_AF5_SPI5}, {GPIOJ, 11, GPIO_AF5_SPI5} },

/*--------- MOSI ---------*/
{ {GPIOF, 9, GPIO_AF5_SPI5}, {GPIOF, 11, GPIO_AF5_SPI5}, {GPIOJ, 10, GPIO_AF5_SPI5} },

};

static const struct pin_node spi6_pinctrl[4][3] = {
/*--------- NSS ---------*/
{ {GPIOG, 8, GPIO_AF5_SPI6}, {GPIOA, 15, GPIO_AF7_SPI6}, {GPIOA, 4, GPIO_AF8_SPI6} },

/*--------- SCK ---------*/
{ {GPIOG, 13, GPIO_AF5_SPI6}, {GPIOA, 5, GPIO_AF8_SPI6}, {GPIOB, 3, GPIO_AF8_SPI6} },

/*--------- MISO ---------*/
{ {GPIOG, 12, GPIO_AF5_SPI6}, {GPIOA, 6, GPIO_AF8_SPI6}, {GPIOB, 4, GPIO_AF8_SPI6} },

/*--------- MOSI ---------*/
{ {GPIOG, 14, GPIO_AF5_SPI6}, {GPIOA, 7, GPIO_AF8_SPI6}, {GPIOB, 5, GPIO_AF8_SPI6} },

};


#define SPI_PINCTRL_SOURCE(NUM, TYPE, SELECT)   \
		(TYPE < SPI_PIN_TYPE_NUM && SELECT <= spi_max_selection_number[NUM - 1]) ?   \
				&spi## NUM ##_pinctrl[INDEX(TYPE)][INDEX(SELECT) - 1] : NULL

/**
 * @brief 
 *  =========================================================================
 * ||																		||
 * ||																		||
 * ||			all qspi device pin resource in  STM32H743IIT6  			||
 * ||																		||
 * ||																		||
 *  =========================================================================
*/
#define		QSPI_PIN_IO_0	 	0
#define		QSPI_PIN_IO_1  		1
#define		QSPI_PIN_IO_2	 	2
#define		QSPI_PIN_IO_3  		3
#define		QSPI_PIN_NCS  		4
#define		QSPI_PIN_TYPE_NUM	5
static const uint8_t qspi_max_selection_number[2] = { 3, 2 };


static const struct pin_node qspi_clk_pinctrl[2] = {
/*--------- CLK ---------*/
{GPIOB, 2, GPIO_AF9_QUADSPI}, {GPIOF, 10, GPIO_AF9_QUADSPI},

};

static const struct pin_node qspi_bank1_pinctrl[5][3] = {
/*--------- IO_0 ---------*/
{ {GPIOC, 9, GPIO_AF9_QUADSPI}, {GPIOD, 11, GPIO_AF9_QUADSPI}, {GPIOF, 8, GPIO_AF10_QUADSPI} },

/*--------- IO_1 ---------*/
{ {GPIOC, 10, GPIO_AF9_QUADSPI}, {GPIOD, 12, GPIO_AF9_QUADSPI}, {GPIOF, 9, GPIO_AF10_QUADSPI} },

/*--------- IO_2 ---------*/
{ {GPIOE, 2, GPIO_AF9_QUADSPI}, {GPIOF, 7, GPIO_AF9_QUADSPI}, {0, 0, 0} },

/*--------- IO_3 ---------*/
{ {GPIOA, 1, GPIO_AF9_QUADSPI}, {GPIOD, 13, GPIO_AF9_QUADSPI}, {GPIOF, 6, GPIO_AF9_QUADSPI} },

/*--------- NCS ---------*/
{ {GPIOB, 10, GPIO_AF9_QUADSPI}, {GPIOB, 6, GPIO_AF10_QUADSPI}, {GPIOG, 6, GPIO_AF10_QUADSPI} },
};

static const struct pin_node qspi_bank2_pinctrl[5][2] = {
/*--------- IO_0 ---------*/
{ {GPIOH, 2, GPIO_AF9_QUADSPI}, {GPIOE, 7, GPIO_AF10_QUADSPI} },

/*--------- IO_1 ---------*/
{ {GPIOH, 3, GPIO_AF9_QUADSPI}, {GPIOE, 8, GPIO_AF10_QUADSPI} },

/*--------- IO_2 ---------*/
{ {GPIOG, 9, GPIO_AF9_QUADSPI}, {GPIOE, 9, GPIO_AF10_QUADSPI}},

/*--------- IO_3 ---------*/
{ {GPIOG, 14, GPIO_AF9_QUADSPI}, {GPIOE, 10, GPIO_AF10_QUADSPI} },

/*--------- NCS ---------*/
{ {GPIOC, 11, GPIO_AF9_QUADSPI}, {0, 0, 0} },
};

#define QSPI_PINCTRL_SOURCE_CLK(SELECT) \
		(SELECT <= 2) ? &qspi_clk_pinctrl[SELECT - 1] : NULL

#define QSPI_PINCTRL_SOURCE(NUM, TYPE, SELECT)   \
		(TYPE < QSPI_PIN_TYPE_NUM && SELECT <= qspi_max_selection_number[NUM - 1]) ?   \
				&qspi_bank## NUM ##_pinctrl[INDEX(TYPE)][INDEX(SELECT) - 1] : NULL
/**
 * @brief 
 *  =========================================================================
 * ||																		||
 * ||																		||
 * ||			all fdcan device pin resource in  STM32H743IIT6  			||
 * ||																		||
 * ||																		||
 *  =========================================================================
*/
#define		CAN_PIN_TX	 		0
#define		CAN_PIN_RX  		1
#define		CAN_PIN_TYPE_NUM	2
static const uint8_t can_max_selection_number[2] = { 5, 2 };

static const struct pin_node can1_pinctrl[2][5] = {
/*--------- TX ---------*/
{ {GPIOA, 12, GPIO_AF9_FDCAN1}, {GPIOB, 9, GPIO_AF9_FDCAN1}, {GPIOD, 1, GPIO_AF9_FDCAN1}, {GPIOH, 13, GPIO_AF9_FDCAN1},
  {0, 0, 0}
},
/*--------- RX ---------*/
{ {GPIOA, 11, GPIO_AF9_FDCAN1}, {GPIOB, 8, GPIO_AF9_FDCAN1}, {GPIOD, 0, GPIO_AF9_FDCAN1}, {GPIOH, 14, GPIO_AF9_FDCAN1}, 
  {GPIOI, 9, GPIO_AF9_FDCAN1}
},
};

static const struct pin_node can2_pinctrl[2][2] = {
/*--------- TX ---------*/
{ {GPIOB, 6, GPIO_AF9_FDCAN2}, {GPIOB, 13, GPIO_AF9_FDCAN2} },
/*--------- RX ---------*/
{ {GPIOB, 5, GPIO_AF9_FDCAN2}, {GPIOB, 12, GPIO_AF9_FDCAN2} },
};

#define CAN_PINCTRL_SOURCE(NUM, TYPE, SELECT)   \
		(TYPE < CAN_PIN_TYPE_NUM && SELECT <= can_max_selection_number[NUM - 1]) ?   \
				&can## NUM ##_pinctrl[INDEX(TYPE)][INDEX(SELECT) - 1] : NULL


/**
 * @brief 
 *  =========================================================================
 * ||																		||
 * ||																		||
 * ||			all sdmmc device pin resource in  STM32H743IIT6  			||
 * ||																		||
 * ||																		||
 *  =========================================================================
*/
#define		SDMMC_D0	 		0
#define		SDMMC_D1	 		1
#define		SDMMC_D2	 		2
#define		SDMMC_D3	 		3
#define		SDMMC_CMD	 		4
#define		SDMMC_CLK	 		5
#define		SDMMC_PIN_TYPE_NUM	6
static const uint8_t sdmmc_max_selection_number[2] = { 1, 2 };

static const struct pin_node sdmmc1_pinctrl[6] = {
/*--------- D0 ---------*/
{GPIOC, 8, GPIO_AF12_SDIO1}, 
/*--------- D1 ---------*/
{GPIOC, 9, GPIO_AF12_SDIO1}, 
/*--------- D2 ---------*/
{GPIOC, 10, GPIO_AF12_SDIO1}, 
/*--------- D3 ---------*/
{GPIOC, 11, GPIO_AF12_SDIO1}, 
/*--------- CMD ---------*/
{GPIOD, 2, GPIO_AF12_SDIO1}, 
/*--------- CLK ---------*/
{GPIOC, 12, GPIO_AF12_SDIO1}, 

};

static const struct pin_node sdmmc2_pinctrl[6][2] = {
/*--------- D0 ---------*/
{ {GPIOB, 14, GPIO_AF9_SDIO2}, {0, 0, 0} },
/*--------- D1 ---------*/
{ {GPIOB, 15, GPIO_AF9_SDIO2}, {0, 0, 0} },
/*--------- D2 ---------*/
{ {GPIOB, 3, GPIO_AF9_SDIO2}, {GPIOG, 11, GPIO_AF10_SDIO2} },
/*--------- D3 ---------*/
{ {GPIOB, 4, GPIO_AF9_SDIO2}, {0, 0, 0} },
/*--------- CMD ---------*/
{ {GPIOA, 0, GPIO_AF9_SDIO2}, {GPIOD, 7, GPIO_AF11_SDIO2} },
/*--------- CLK ---------*/
{ {GPIOC, 1, GPIO_AF9_SDIO2}, {GPIOD, 6, GPIO_AF11_SDIO2} },
};

#define SDMMC1_PINCTRL_SOURCE(TYPE)   \
		(TYPE < SDMMC_PIN_TYPE_NUM) ? &sdmmc1_pinctrl[INDEX(TYPE)] : NULL

#define SDMMC2_PINCTRL_SOURCE(TYPE, SELECT)   \
		(TYPE < SDMMC_PIN_TYPE_NUM && SELECT <= sdmmc_max_selection_number[1]) ?   \
				&sdmmc2_pinctrl[INDEX(TYPE)][INDEX(SELECT) - 1] : NULL



/**
 * @brief 
 *  =========================================================================
 * ||																		||
 * ||																		||
 * ||			all i2c device pin resource in  STM32H743IIT6  				||
 * ||																		||
 * ||																		||
 *  =========================================================================
*/
#define		I2C_PIN_SCL	 		0
#define		I2C_PIN_SDA  		1
#define		I2C_PIN_TYPE_NUM	2
static const uint8_t i2c_max_selection_number[4] = { 2, 3, 2, 5 };

static const struct pin_node i2c1_pinctrl[2][5] = {
/*--------- SCL ---------*/
{ {GPIOB, 6, GPIO_AF4_I2C1}, {GPIOB, 8, GPIO_AF4_I2C1}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}
},
/*--------- SDA ---------*/
{ {GPIOB, 7, GPIO_AF4_I2C1}, {GPIOB, 9, GPIO_AF4_I2C1}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}
},
};

static const struct pin_node i2c2_pinctrl[2][5] = {
/*--------- SCL ---------*/
{ {GPIOB, 10, GPIO_AF4_I2C2}, {GPIOF, 1, GPIO_AF4_I2C2}, {GPIOH, 5, GPIO_AF4_I2C2}, {0, 0, 0}, {0, 0, 0}
},

/*--------- SDA ---------*/
{ {GPIOB, 11, GPIO_AF4_I2C2}, {GPIOF, 0, GPIO_AF4_I2C2}, {GPIOH, 5, GPIO_AF4_I2C2}, {0, 0, 0}, {0, 0, 0}
},

};

static const struct pin_node i2c3_pinctrl[2][5] = {
/*--------- SCL ---------*/
{ {GPIOA, 8, GPIO_AF4_I2C3}, {GPIOH, 7, GPIO_AF4_I2C3}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}
},
/*--------- SDA ---------*/
{ {GPIOC, 9, GPIO_AF4_I2C3}, {GPIOH, 8, GPIO_AF4_I2C3}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}
},
};

static const struct pin_node i2c4_pinctrl[2][5] = {
/*--------- SCL ---------*/
{ {GPIOB, 6, GPIO_AF6_I2C4}, {GPIOB, 8, GPIO_AF6_I2C4}, {GPIOD, 12, GPIO_AF4_I2C4}, 
  {GPIOF, 14, GPIO_AF4_I2C4}, {GPIOH, 11, GPIO_AF4_I2C4}
},
/*--------- SDA ---------*/
{ {GPIOB, 7, GPIO_AF6_I2C4}, {GPIOB, 9, GPIO_AF6_I2C4}, {GPIOD, 13, GPIO_AF4_I2C4}, 
  {GPIOF, 15, GPIO_AF4_I2C4}, {GPIOH, 12, GPIO_AF4_I2C4}
},
};


#define I2C_PINCTRL_SOURCE(NUM, TYPE, SELECT)   \
		(TYPE < CAN_PIN_TYPE_NUM && SELECT <= i2c_max_selection_number[NUM - 1]) ?   \
				&i2c## NUM ##_pinctrl[INDEX(TYPE)][INDEX(SELECT) - 1] : NULL



#endif


