#include <p18f4550.h>
#include "tlc5940.h"
#include <timers.h>
#include <adc.h>
#include "usb_functions.h"

char tamanhoDeExpansao = '-'; //Variavel que armazena o tamanho da expansão

void high_isr(void);
#pragma code high_vector=0x08
 void interrupt_at_high_vector(void)
 {
  _asm GOTO high_isr _endasm
 }

#pragma code

#pragma interrupt high_isr
 void high_isr(void)
 {
     if(INTCONbits.TMR0IF)
     {
         WriteTimer0(XLATCOUNTTIMER); //23531 = 500ms
         processXLATinterrupt();
         INTCONbits.TMR0IF=0;
     }
}

int processa_controle( char controle );
int processa_dado( char dado );
void rotina_usb();
void inicia_motores();
void config_pic();
int config_expansao();
void ativa_dados( char dados[30]);

void main(void)
{
    config_pic();
    initialiseTlc5940();
    config_expansao(); //Verificação de existencia de expansão
    usb_install();
    inicia_motores(0,64,3891); // Configura os motores inicialmente p/ posição zero

    do
    {
        rotina_usb();
    } while(1);
}



void rotina_usb()
{
        char byte_recebido;
        char mascara_tipo_byte = 0xC0; // 0b11000000
        char mascara_valor_byte = 0x3F; // 0b00111111
        char tipo_byte;
        char valor_byte;
        
        usb_handler();

        /* O valor 0xFF funciona como padrao para a variavel byte_recebido,
         * e significa que nenhum byte foi recebido pelo PIC. */
        byte_recebido = 0xFF;

        /* Caso tenha algum char no buffer de entrada do PIC,
         * esse sera' transferido para a variavel byte_recebido.
         * Caso nao, o valor inicial 0xFF sera' mantido. */
        poll_getc_cdc(&byte_recebido);

        /* Se nenhum byte tiver sido recebido, esse ciclo do loop
         * sera' encerrado nesse ponto. */
        if ( byte_recebido == 0xFF ) return;

        processa_controle(byte_recebido);
}



int processa_controle( char controle )
{
     char mascara_tipo_byte = 0x5A; // Caracter 'Z' será verificação da Expansao (ADC)
     /* O comando putc_cdc envia o byte controle para o buffer de saida do
     * PIC. Nesse caso, esse byte sera diferente do enviado para o PIC, pois
     * os dois bits mais significativos foram transformados em 00 e, se fossem
     * iguais a 00 inicialmente, essa funcao nao teria sido chamada. */
    if(controle ==  mascara_tipo_byte)
    {
        putc_cdc(' ');
        putc_cdc(tamanhoDeExpansao);
        putc_cdc(' ');
        inicia_motores(0,64,3687);
    }
    else if(controle == 0x30) // Caracter '0' será dados
    {
        processa_dado(0);
    }
    else
    {
        inicia_motores(0,64,3687);
        putc_cdc('c');
    }
    return 1;
}



int processa_dado( char dado )
{
    /* Ja' nesse caso, o byte enviado sera identico ao recebido. */
    char data[10];
    data[0]='?';//? = 0b00111111
    data[1]='0';//0 = 0b00110000
    data[2]='0';
    data[3]='0';
    data[4]='0';
    data[5]='0';
    data[6]='0';
    data[7]='0';
    data[8]='0';
    data[9]='?';
    ativa_dados(data);
    putc_cdc(' ');
    putc_cdc('d');

    return 1;
}



void inicia_motores( unsigned char inicial, unsigned char final, int quanto )
{
    unsigned char canal;
    for(canal = inicial; canal < final; canal++)
    {
      setGrayScaleValue(canal, quanto);
    }
    updateTlc5940();
    PORTDbits.RD7 = 1;
}



void config_pic(void)
{
    OpenADC(ADC_FOSC_8 & ADC_RIGHT_JUST & ADC_20_TAD, ADC_CH1 & ADC_INT_OFF & ADC_VREFPLUS_VDD & ADC_VREFMINUS_VSS, ADC_2ANA); //Configura o canal analógico AN0
    
    TRISA = 0x0F;
    TRISB = 0x00;
    TRISC = 0x00;
    TRISD = 0x00;
    TRISE = 0x00;
    PORTA = 0x00;
    PORTB = 0x00;
    PORTC = 0x00;
    PORTD = 0x00;
    PORTE = 0x00;

    ADCON1 = 0x0D; // 0b0000 1011 - Port A0 até A3 como analógico (Bit 3,2,1 e 0) Setado como +5 e 0V (Bit 6 e 5)
    SSPCON1bits.SSPEN = 1;
}



int config_expansao(void)
{
    unsigned short tensao; // Armazena o valor da conversão ADC feita
    unsigned short numeroDeVericacao=0; // Sinalização de erro de leitura

    while(1)
    {
        SetChanADC(ADC_CH1); // Seta o canal analógico 1 (AN1) no qual verifica a existensia ou não de expansão por um divisor de tensão
        ConvertADC(); //Inicia conversão ADC
        while(BusyADC()); // Aguarda a finalização da conversão
        tensao = ReadADC(); //Guarda a informação obtida da conversão
        /* * Funsão desenvolvida para verificar o nível de tensão no Port AN0, no qual verificará se existe expansão conectada
                 e o tamanho da mesma, de acordo com os seguintes parametros:

                  -> Sem Expansão:
                  Nível de tensão superiores a 3,5V. Idealmente deve estar em 4,55V (Conforme divisor de tensão utilizado)

                  -> Expansão de 10 Celulas:
                  Nivel de tensão inferiores a 1,0V. Idealmente deve estar em 0V (Conforme divisor de tensão utilizado)

                  -> Expansão de 20 Celulas:
                  Nível de tensão na faixa de 1,5 a 3,0V. Idealmente deve estar em 2,3V (Conforme divisor de tensão utilizado)

                  -> Região de incerteza/Erro de leitura:
                  Nível de tensão nas faixas de 1,0 a 1,5V e 3,0 a 3,5V;
                  Para essa situação, o software irá reverificar o nível de tensão novamente, caso caia na região de incerteza por
                  três vezes seguidas, irá selecionar o módulo Sem Expansão e informar o erro ao Alto Nível;

        */
        // Sem Expanssão (10 Celulas)
        if (tensao >= 717) //Verifica se a tensão da porta AN0 é maior que 3,5V (717/1024*5V=3,501V)
        {
            if(tamanhoDeExpansao = '0')
            {
                return 0;
            }
            tamanhoDeExpansao = '0';
        }
        // Expanssão no tamanho pequeno (10 + 10 Celulas = 20 )
        else if (tensao <= 205) //Verifica se a tensão da porta AN0 é menor que 1V (205/1024*5V=1,001V) (O ideal é que na porta tenha 0V)
        {
            if(tamanhoDeExpansao = '1')
            {
                return 0;
            }
            tamanhoDeExpansao = '1';
        }
        // Expanssão no tamanho máximo (10 + 20 Celulas)
        else if (tensao >= 307 && tensao <= 614) //Verifica se a tensão da porta AN0 é maior que 1,5V (307/1024*5=1,499V) e menor que 3V (614/1024*5=2,998V)
        {
            if(tamanhoDeExpansao = '2')
            {
                return 0;
            }
            tamanhoDeExpansao = '2';
        }
        // Erro de leitura
        if(numeroDeVericacao > 1) //Caso o processo faça mais que 3 mediçoes no ADC sem fazer 2 medidas consecutivas iguais, será informado erro
        {
            tamanhoDeExpansao = 'X';
            return 0;
        }
        numeroDeVericacao++;
    }
}



void ativa_dados( char dados[10])
{
    int valor;
    char mascara;
    unsigned short brightness1=3800;
    unsigned short brightness2=3700;
    unsigned short celula = 0;

    putc_cdc(' ');
    while(celula<10)
    {
        mascara = 0b00100000;
        valor=0;
        while (mascara != 0b00000000)
        {
            if((dados[celula] & mascara) == 0b00000000)
            {
                setGrayScaleValue(celula*6+valor, brightness1);
                putc_cdc('0');
            }
            else
            {
                setGrayScaleValue(celula*6+valor, brightness2);
                putc_cdc('1');
            }
            mascara = mascara >> 1;
            valor++;
        }
        celula++;
        putc_cdc(' ');
    }
    updateTlc5940();
}