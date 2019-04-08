//IOS
//OLEDBoard
#define BUT1_PIO		  PIOD
#define BUT1_MASK		  (1u << 28u)

#define BUT2_PIO		  PIOC
#define BUT2_MASK		  (1u << 31)

#define BUT3_PIO		  PIOA
#define BUT3_MASK		  (1u << 19)

//Onboard LED
#define  LED_PIO  PIOC
#define  LED_MASK (1u << 8u)


void io_init(void);