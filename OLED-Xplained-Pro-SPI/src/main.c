#include <asf.h>

#include "gfx_mono_ug_2832hsweg04.h"
#include "gfx_mono_text.h"
#include "sysfont.h"



// LED
#define LED_PIO      PIOC
#define LED_PIO_ID   ID_PIOC
#define LED_IDX      8
#define LED_IDX_MASK (1 << LED_IDX)

// Botão
#define BUT_PIO      PIOA
#define BUT_PIO_ID   ID_PIOA
#define BUT_IDX  11
#define BUT_IDX_MASK (1 << BUT_IDX)

#define BUT1_PIO	PIOD
#define BUT1_PIO_ID	 ID_PIOD
#define BUT1_PIO_IDX 28
#define BUT1_PIO_IDX_MASK (1u << BUT1_PIO_IDX)


/************************************************************************/
/* constants                                                            */
/************************************************************************/

/************************************************************************/
/* variaveis globais                                                    */
/************************************************************************/
volatile char but_flag = 0;
volatile int tempo = 0;
char buffer[128];
bool apertado = false;
int button_down_current = 0;

/************************************************************************/
/* prototype                                                            */
/************************************************************************/
void io_init(void);
void pisca_led(int n, int t);
void atualiza_frequencia();

/************************************************************************/
/* functions                                                            */
/************************************************************************/

void but_raise(void) {
	apertado = !apertado;
	but_flag = 1;
}

// pisca led N vez no periodo T
void pisca_led(int n, int t){
	pio_set(LED_PIO, LED_IDX_MASK);
	delay_ms(t);
	pio_clear(LED_PIO, LED_IDX_MASK);
	delay_ms(t);
}


void atualiza_frequencia() {
	snprintf(buffer, sizeof(buffer) / sizeof(char), "%f", ((float)tempo / (float)1000));
}

// Inicializa botao SW0 do kit com interrupcao
void io_init(void)
{
	board_init();
	sysclk_init();
	delay_init();
	WDT->WDT_MR = WDT_MR_WDDIS;

	// Init OLED
	gfx_mono_ssd1306_init();

	// Configura led
	pmc_enable_periph_clk(LED_PIO_ID);
	pmc_enable_periph_clk(BUT1_PIO_ID);
	//pmc_enable_periph_clk(BUT2_PIO_ID);
	//pmc_enable_periph_clk(BUT3_PIO_ID);
	
	pio_configure(LED_PIO, PIO_OUTPUT_0, LED_IDX_MASK, PIO_DEFAULT);
	pio_configure(BUT1_PIO, PIO_INPUT, BUT1_PIO_IDX_MASK, PIO_DEFAULT);

	// Inicializa clock do periférico PIO responsavel pelo botao
	pmc_enable_periph_clk(BUT1_PIO_ID);

	// Configura PIO para lidar com o pino do botão como entrada
	// com pull-up
	pio_configure(BUT1_PIO, PIO_INPUT, BUT1_PIO_IDX_MASK, PIO_PULLUP | PIO_DEBOUNCE);
	pio_set_debounce_filter(BUT1_PIO, BUT1_PIO_IDX_MASK, 60);

	// Configura interrupção no pino referente ao botao e associa
	// função de callback caso uma interrupção for gerada
	// a função de callback é a: but_callback()
	//pio_handler_set(
		//BUT1_PIO,
		//BUT1_PIO_ID,
		//BUT1_PIO_IDX_MASK,
		//PIO_IT_FALL_EDGE,
		//but_fall);
					
	pio_handler_set(
		BUT1_PIO,
		BUT1_PIO_ID,
		BUT1_PIO_IDX_MASK,
		PIO_IT_EDGE,
		but_raise);


	// Ativa interrupção e limpa primeira IRQ gerada na ativacao
	pio_enable_interrupt(BUT1_PIO, BUT1_PIO_IDX_MASK);
	pio_get_interrupt_status(BUT1_PIO);
	
	// Configura NVIC para receber interrupcoes do PIO do botao
	// com prioridade 4 (quanto mais próximo de 0 maior)
	NVIC_EnableIRQ(BUT1_PIO_ID);
	NVIC_SetPriority(BUT1_PIO_ID, 4); // Prioridade 4
}

int main(void) {
	io_init();
	but_flag = 0;
	tempo = 1000;
	buffer[0] = '\0';

int n = 0;
	while (1) {
		if (apertado) {
			button_down_current++;
		}
		else {
			if (button_down_current > 0){
				if (button_down_current >= 3){
					tempo -= 100;
				}
				else{
					tempo += 100;
				}
				sprintf(buffer, "%d %d %d ", button_down_current, apertado, tempo);
				gfx_mono_draw_string(buffer, 0, 16, &sysfont);
				button_down_current = 0;
			}
		}
		if(but_flag){
			pisca_led(30, tempo);
		}

		// Handle LED blinking and other tasks here

		//pmc_sleep(SAM_PM_SMODE_SLEEP_WFI);
		//gfx_mono_draw_string(buffer, 50, 16, &sysfont);
	}
}
