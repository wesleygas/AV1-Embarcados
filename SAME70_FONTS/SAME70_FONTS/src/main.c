/*
 * main.c
 *
 * Created: 05/03/2019 18:00:58
 *  Author: eduardo
 */ 

#include <asf.h>
#include "tfont.h"
#include "sourcecodepro_28.h"
#include "calibri_36.h"
#include "arial_72.h"


struct ili9488_opt_t g_ili9488_display_opt;
#define YEAR	2019
#define MONTH	03
#define DAY		30
#define WEEK	12
//Time
#define HOUR	11
#define MINUTE	59
#define SECOND	50
#define wheel_diameter 0.65
#define PI		3.141592
typedef struct Horario{
	int hora;
	int minuto;
	int segundo;
} Horario;

void RTC_Handler(void);
void timeToString(char *str, Horario tempo);
void calcTimeDiff(Horario curr_time, Horario est_finish, Horario *eta);
void RTC_init(void);
void pin_toggle(Pio *pio, uint32_t mask);


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

//Var Globais
volatile int roda_voltas = 0;
volatile int update_vel_alarm = 1;
volatile int update_time = 0;

void RTT_Handler(void)
{
	uint32_t ul_status;

	/* Get RTT status */
	ul_status = rtt_get_status(RTT);

	/* IRQ due to Time has changed */
	if ((ul_status & RTT_SR_RTTINC) == RTT_SR_RTTINC) {  }

	/* IRQ due to Alarm */
	if ((ul_status & RTT_SR_ALMS) == RTT_SR_ALMS) {
		update_vel_alarm = 1;                  // flag RTT alarme
	}
}

void RTC_Handler(void)
{
	uint32_t ul_status = rtc_get_status(RTC);

	/*
	*  Verifica por qual motivo entrou
	*  na interrupcao, se foi por segundo
	*  ou Alarm
	*/
	if ((ul_status & RTC_SR_SEC) == RTC_SR_SEC) {
		rtc_clear_status(RTC, RTC_SCCR_SECCLR);
		update_time = 1;
		pin_toggle(LED_PIO,LED_MASK);
	}
	
	/* Time or date alarm */
	if ((ul_status & RTC_SR_ALARM) == RTC_SR_ALARM) {
		rtc_clear_status(RTC, RTC_SCCR_ALRCLR); //Avisa q foi handled
			
	}
	
	rtc_clear_status(RTC, RTC_SCCR_ACKCLR);
	rtc_clear_status(RTC, RTC_SCCR_TIMCLR);
	rtc_clear_status(RTC, RTC_SCCR_CALCLR);
	rtc_clear_status(RTC, RTC_SCCR_TDERRCLR);
	
}

void configure_lcd(void){
	/* Initialize display parameter */
	g_ili9488_display_opt.ul_width = ILI9488_LCD_WIDTH;
	g_ili9488_display_opt.ul_height = ILI9488_LCD_HEIGHT;
	g_ili9488_display_opt.foreground_color = COLOR_CONVERT(COLOR_WHITE);
	g_ili9488_display_opt.background_color = COLOR_CONVERT(COLOR_WHITE);

	/* Initialize LCD */
	ili9488_init(&g_ili9488_display_opt);
	ili9488_draw_filled_rectangle(0, 0, ILI9488_LCD_WIDTH-1, ILI9488_LCD_HEIGHT-1);
	
}

void pin_toggle(Pio *pio, uint32_t mask){
	if(pio_get_output_data_status(pio, mask))
	pio_clear(pio, mask);
	else
	pio_set(pio,mask);
}

void magsense_callback(void){
	roda_voltas++;
	pin_toggle(LED_PIO,LED_MASK);
}

void io_init(void){
	pmc_enable_periph_clk(ID_PIOA);
	//pmc_enable_periph_clk(ID_PIOB);
	pmc_enable_periph_clk(ID_PIOC);
	pmc_enable_periph_clk(ID_PIOD);
	
	pio_configure(BUT3_PIO, PIO_INPUT,BUT3_MASK,PIO_DEBOUNCE|PIO_PULLUP);
	pio_configure(BUT2_PIO, PIO_INPUT,BUT2_MASK,PIO_DEBOUNCE|PIO_PULLUP);
	pio_configure(BUT1_PIO, PIO_INPUT,BUT1_MASK,PIO_DEBOUNCE|PIO_PULLUP);
	
	pio_configure(LED_PIO, PIO_OUTPUT_0, LED_MASK, PIO_DEFAULT);
	
	//pio_handler_set(BUT1_PIO,ID_PIOD,BUT1_MASK,PIO_IT_FALL_EDGE, magsense_callback);
	pio_handler_set(BUT3_PIO,ID_PIOA,BUT3_MASK,PIO_IT_FALL_EDGE, magsense_callback);
	// Ativa interrupção no hardware
	//pio_enable_interrupt(PIOD, BUT_MASK);
	pio_enable_interrupt(PIOA, BUT3_MASK);

	// Configura NVIC para receber interrupcoes do PIO do botao
	// com prioridade 4 (quanto mais próximo de 0 maior)
	NVIC_EnableIRQ(ID_PIOA);
	NVIC_SetPriority(ID_PIOA, 3); // Prioridade 4
}

static void RTT_init(uint16_t pllPreScale, uint32_t IrqNPulses)
{
	uint32_t ul_previous_time;

	/* Configure RTT for a 1 second tick interrupt */
	rtt_sel_source(RTT, false);
	rtt_init(RTT, pllPreScale);
	
	ul_previous_time = rtt_read_timer_value(RTT);
	while (ul_previous_time == rtt_read_timer_value(RTT));
	
	rtt_write_alarm_time(RTT, IrqNPulses+ul_previous_time);

	/* Enable RTT interrupt */
	NVIC_DisableIRQ(RTT_IRQn);
	NVIC_ClearPendingIRQ(RTT_IRQn);
	NVIC_SetPriority(RTT_IRQn, 4);
	NVIC_EnableIRQ(RTT_IRQn);
	rtt_enable_interrupt(RTT, RTT_MR_ALMIEN);
}

static void rtt_reconfigure(){
	
      /*
       * O clock base do RTT é 32678Hz
       * Para gerar outra base de tempo é necessário
       * usar o PLL pre scale, que divide o clock base.
       *
       * Nesse exemplo, estamos operando com um clock base
       * de pllPreScale = 32768/32768/2 = 2Hz
       *
       * Quanto maior a frequência maior a resolução, porém
       * menor o tempo máximo que conseguimos contar.
       *
       * Podemos configurar uma IRQ para acontecer quando 
       * o contador do RTT atingir um determinado valor
       * aqui usamos o irqRTTvalue para isso.
       * 
       * Nesse exemplo o irqRTTvalue = 8, causando uma
       * interrupção a cada 2 segundos (lembre que usamos o 
       * pllPreScale, cada incremento do RTT leva 500ms (2Hz).
       */
      uint16_t pllPreScale = (int) (((float) 32768) / 1.0);
      uint32_t irqRTTvalue  = 4;
      
      // reinicia RTT para gerar um novo IRQ
      RTT_init(pllPreScale, irqRTTvalue);         
      
     /*
      * caso queira ler o valor atual do RTT, basta usar a funcao
      *   rtt_read_timer_value()
      */
      
      /*
       * CLEAR FLAG
       */
}

/**
* Configura o RTC para funcionar com interrupcao de alarme
*/
void RTC_init(){
	/* Configura o PMC */
	pmc_enable_periph_clk(ID_RTC);

	/* Default RTC configuration, 24-hour mode */
	rtc_set_hour_mode(RTC, 0);

	/* Configura data e hora manualmente */
	rtc_set_date(RTC, YEAR, MONTH, DAY, WEEK);
	rtc_set_time(RTC, HOUR, MINUTE, SECOND);

	/* Configure RTC interrupts */
	NVIC_DisableIRQ(RTC_IRQn);
	NVIC_ClearPendingIRQ(RTC_IRQn);
	NVIC_SetPriority(RTC_IRQn, 2);
	NVIC_EnableIRQ(RTC_IRQn);

	/* Ativa interrupcao via alarme */
	rtc_enable_interrupt(RTC,  RTC_IER_SECEN);

}

void timeToString(char *str, Horario tempo){
	if(tempo.hora < 10){
		str[0] = '0';
		str[1] = tempo.hora + 48;
		}else{
		str[0] = tempo.hora/10 + 48;
		str[1] = tempo.hora%10 + 48;
	}
	str[2] = ':';
	if(tempo.minuto < 10){
		str[3] = '0';
		str[4] = tempo.minuto + 48;
		}else{
		str[3] = tempo.minuto/10 + 48;
		str[4] = tempo.minuto%10 + 48;
	}
	str[5] = ':';
	if(tempo.segundo < 10){
		str[6] = '0';
		str[7] = tempo.segundo + 48;
		}else{
		str[6] = tempo.segundo/10 + 48;
		str[7] = tempo.segundo%10 + 48;
	}
	str[8] = 0;
	
}


/*
* Calculate time difference: est_finish - curr_time = eta
*/
void calcTimeDiff(Horario curr_time, Horario est_finish, Horario *eta){
	if(curr_time.segundo > est_finish.segundo){
		est_finish.minuto --;
		est_finish.segundo += 60;
	}
	eta->segundo = est_finish.segundo - curr_time.segundo;
	
	if(curr_time.minuto > est_finish.minuto){
		est_finish.hora --;
		est_finish.minuto += 60;
	}
	eta->minuto = est_finish.minuto - curr_time.minuto;
	eta->hora = est_finish.hora - curr_time.hora;
}

void font_draw_text(tFont *font, const char *text, int x, int y, int spacing) {
	char *p = text;
	while(*p != NULL) {
		char letter = *p;
		int letter_offset = letter - font->start_char;
		if(letter <= font->end_char) {
			tChar *current_char = font->chars + letter_offset;
			ili9488_draw_pixmap(x, y, current_char->image->width, current_char->image->height, current_char->image->data);
			x += current_char->image->width + spacing;
		}
		p++;
	}	
}


int main(void) {
	board_init();
	sysclk_init();	
	configure_lcd();
	RTC_init();
	io_init();
	Horario curr_time;
	Horario initial_time;
	Horario time_diff;
	rtc_get_time(RTC,&initial_time.hora, &initial_time.minuto, &initial_time.segundo);
	float angular_vel = 0;
	float inst_vel = 0;
	float total_dist = 0;
	char buff_str[30];
	char time_str[9];
	//font_draw_text(&sourcecodepro_28, "OIMUNDO", 50, 50, 1);
	//font_draw_text(&calibri_36, "Oi Mundo! #$!@", 50, 100, 1);
	//font_draw_text(&arial_72, "102456", 50, 200, 2);
	while(1) {
		if(update_vel_alarm){
			angular_vel = (float)2*PI*roda_voltas/4;
			inst_vel = angular_vel*wheel_diameter*3.6;
			total_dist += roda_voltas*2*PI*wheel_diameter;
			roda_voltas = 0;
			sprintf(buff_str,"%.2f %.2f",total_dist, inst_vel);
			font_draw_text(&calibri_36, buff_str, 50, 150, 1);
			rtt_reconfigure();
			update_vel_alarm = 0;
		}if(update_time){
			rtc_get_time(RTC,&curr_time.hora, &curr_time.minuto, &curr_time.segundo);
			//timeToString(time_str,curr_time);
			//font_draw_text(&calibri_36, time_str, 50, 250, 1);
			calcTimeDiff(initial_time,curr_time, &time_diff);
			timeToString(time_str,time_diff);
			font_draw_text(&calibri_36, time_str, 50, 300, 1);
		}
	}
}