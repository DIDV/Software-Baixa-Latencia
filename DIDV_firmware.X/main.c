#include <p18f4550.h>
#include "tlc5940.h"
#include <timers.h>
#include <adc.h>
#include "usb_functions.h"

char tamanhoDeExpansao = '-'; //Variavel que armazena o tamanho da expans�o

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
void mexe();
void config_pic();
char config_expansao( char leitura );
void ativa_dados( char dados[30]);

void main(void)
{
    config_pic();
    initialiseTlc5940();
    config_expansao('E'); //escrita
    usb_install();
    mexe(0,64,3891);

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

        /* A variavel tipo_byte determinara se o byte conte'm um dado (00),
         * que deve ser enviado para os caracteres braille, ou um
         * controle, que deve ser interpretado e executado pelas funcoes
         * adequadas. */
        tipo_byte = byte_recebido & mascara_tipo_byte;

        /* A variavel valor_byte contem o restante do byte, ignorando os dois
         * bits de controle. Exceto por uma questao de padrao, e' meio inutil,
         * na verdade. */
        valor_byte = byte_recebido & mascara_valor_byte;

        if ( tipo_byte ) // se tipo_byte for diferente de zero...
            processa_controle(byte_recebido);
        else
            processa_dado(valor_byte);
}



int processa_controle( char controle )
{
     char mascara_tipo_byte = 0x5A; // Caracter 'Z'
     /* O comando putc_cdc envia o byte controle para o buffer de saida do
     * PIC. Nesse caso, esse byte sera diferente do enviado para o PIC, pois
     * os dois bits mais significativos foram transformados em 00 e, se fossem
     * iguais a 00 inicialmente, essa funcao nao teria sido chamada. */
    if(controle ==  mascara_tipo_byte)
    {
        putc_cdc(' ');
        putc_cdc(config_expansao('L'));
        putc_cdc(' ');
        mexe(0,64,3687);
    }
    else
    {
        mexe(0,64,3687);
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



void mexe( unsigned char inicial, unsigned char final, int quanto )
{
    unsigned char canal;
    for(canal = inicial; canal < final; canal++)
    {
      setGrayScaleValue(canal, quanto);
    } updateTlc5940();

}



void config_pic(void)
{
    OpenADC(ADC_FOSC_8 & ADC_RIGHT_JUST & ADC_20_TAD, ADC_CH1 & ADC_INT_OFF & ADC_VREFPLUS_VDD & ADC_VREFMINUS_VSS, ADC_2ANA); //Configura o canal anal�gico AN0
    
    TRISA = 0x0F;
    TRISB = 0x00;
    TRISC = 0x00;
    TRISE = 0x00;
    PORTA = 0x00;
    PORTB = 0x00;
    PORTC = 0x00;
    PORTE = 0x00;

    ADCON1 = 0x0D; // 0b0000 1011 - Port A0 at� A3 como anal�gico (Bit 3,2,1 e 0) Setado como +5 e 0V (Bit 6 e 5)
    SSPCON1bits.SSPEN = 1;
}



char config_expansao( char leitura )
{
    //char tamanhoDeExpansao; //Variavel que armazena o tamanho da expans�o

    if(leitura == 'E') // Processo que descobre o tamanho da expans�o
    {
        unsigned short tensao; // Armazena o valor da convers�o ADC feita
        unsigned short erroDeLeitura=0; // Sinaliza��o de erro de leitura

        while(1)
        {
            SetChanADC(ADC_CH1); // Seta o canal anal�gico 1 (AN1) no qual verifica a existensia ou n�o de expans�o por um divisor de tens�o
            ConvertADC(); //Inicia convers�o ADC
            while(BusyADC()); // Aguarda a finaliza��o da convers�o
            tensao = ReadADC(); //Guarda a informa��o obtida da convers�o

            /* * Funs�o desenvolvida para verificar o n�vel de tens�o no Port AN0, no qual verificar� se existe expans�o conectada
                 e o tamanho da mesma, de acordo com os seguintes parametros:

                  -> Sem Expans�o:
                  N�vel de tens�o superiores a 3,5V. Idealmente deve estar em 4,55V (Conforme divisor de tens�o utilizado)

                  -> Expans�o de 10 Celulas:
                  Nivel de tens�o inferiores a 1,0V. Idealmente deve estar em 0V (Conforme divisor de tens�o utilizado)

                  -> Expans�o de 20 Celulas:
                  N�vel de tens�o na faixa de 1,5 a 3,0V. Idealmente deve estar em 2,3V (Conforme divisor de tens�o utilizado)

                  -> Regi�o de incerteza:
                  N�vel de tens�o nas faixas de 1,0 a 1,5V e 3,0 a 3,5V;
                  Para essa situa��o, o software ir� reverificar o n�vel de tens�o novamente, caso caia na regi�o de incerteza por
                  tr�s vezes seguidas, ir� selecionar o m�dulo Sem Expans�o e informar o erro ao Alto N�vel;

            */
            // Sem Expanss�o (10 Celulas)
            if (tensao >= 717) //Verifica se a tens�o da porta AN0 � maior que 3,5V (717/1024*5V=3,501V)
            {
                tamanhoDeExpansao = '0';
                return '0';
            }
            // Expanss�o no tamanho pequeno (10 + 10 Celulas = 20 )
            else if (tensao <= 205) //Verifica se a tens�o da porta AN0 � menor que 1V (205/1024*5V=1,001V) (O ideal � que na porta tenha 0V)
            {
                tamanhoDeExpansao = '1';
                return '1';
            }
            // Expanss�o no tamanho m�ximo (10 + 20 Celulas)
            else if (tensao >= 307 && tensao <= 614) //Verifica se a tens�o da porta AN0 � maior que 1,5V (307/1024*5=1,499V) e menor que 3V (614/1024*5=2,998V)
            {
                tamanhoDeExpansao = '2';
                return '2';
            }
            // Regi�o de incerteza
            // O ideal � que esta tens�o seja de 2,5V na Porta (Talvez colocar regi�es de incerteza (menor que 1V (Ideal 0V) Exp10; 1-1,5V Incerteza; 1,5-3V EXP20 (Ideal 2,3V); 3-3,5V Incerteza; Maior q 3,5V Sem EXP (Ideal 4,5V);
            if(erroDeLeitura=2)//Na terceira vez que ocorre a leitura, sem que haja um valor correto de expans�o
            {
                tamanhoDeExpansao = 'X';
                return 'X';
            }
            erroDeLeitura++;
        }
    }
    else // Leitura do tamanho da expans�o
    {
        return tamanhoDeExpansao;
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