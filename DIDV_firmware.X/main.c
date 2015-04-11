#include <p18f4550.h>
#include "tlc5940.h"
#include <timers.h>
#include <adc.h>
#include "usb_functions.h"

//\TODO: modificar para valores reais
#define PINO_ALTO 3700  // Define o valor do PWM para nível lógico alto;
#define PINO_BAIXO 3800 // Define o valor do PWM para nível lógico baixo;

char tamanhoDeExpansao = '-'; // Variavel que armazena o tamanho da expansão

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
char recebe_dado_usb();
void inicia_motores();
void config_pic();
void config_expansao();
void ativa_dados(char dados[10]);

void main(void)
{
    char byte_recebido;
    config_pic();                       // Configurações iniciais do PIC (Ports, etc.);
    initialiseTlc5940();                // Configuração do sistema de acionamento dos drivers;
    config_expansao();                  // Verificação de existencia de expansão;
    usb_install();                      // Inicialização do USB;
    inicia_motores(0,64,3891);          // Configura os motores inicialmente p/ posição zero;
    Delay_ms(3);                        // Aguarda um tempo antes de acionar o rele de alimentação dos motores
    PORTDbits.RD7 = 1;                  // Bit de acionamento de rele (alimentação dos motores)

    do
    {
        byte_recebido = recebe_dado_usb();  // Recebe os dados vindos da rasp;
        processa_controle(byte_recebido);   // Envia o dado obtido para que o controle tome uma decisão;
    } while(1);
}


/* Função no qual tem como objetivo buscar os dados no buffer de entrada do
 PIC, só retornando dela quando alguma informação for recebida*/
char recebe_dado_usb()
{
    char byte_recebido;

    do
    {
        /*Maquina de estados do USB, no qual verifica se o mesmo esta configurado
         e executa a inclusão e exclusão de dados nos buffer de entrada e saída.*/
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
    } while ( byte_recebido == 0xFF );

    return byte_recebido;                   // Retorna o dado recebido;

} 




/* Função de controle, no qual toma as decisões do que será feito de acordo
 com o comando recebido da Rasp;*/
int processa_controle( char controle )
{
    switch(controle)
    {
        case 0x40:              // Caracter '@' ou 0x40 - Processo de recebimento de dados;
            processa_dado('@');
            break;

        case 0x5F:              // Caracter '_' ou 0x5F - Processo de dados teste pré-estabelecidos;
            processa_dado('_');
            break;

        case 0x51:              //Caracter 'Q' ou 0x51 - Processo de verificação do tamanho da expansão;
            /* O comando putc_cdc envia o byte controle para o buffer de saida do
             * PIC. Nesse caso, esse byte sera diferente do enviado para o PIC, pois
             * os dois bits mais significativos foram transformados em 00 e, se fossem
             * iguais a 00 inicialmente, essa funcao nao teria sido chamada. */
            putc_cdc(tamanhoDeExpansao);
            putc_cdc('Q');      // Test Point
            break;

        case 0x45:              // Caracter 'E' ou 0x45 - Processo de reconfiguração de expansão;
            config_expansao();
            putc_cdc('E');      // Test Point
            break;

        case 0x48:              // Caracter 'H' ou 0x48 - Coloca todos os motores em nível alto;
            inicia_motores(0,64,3687);
            putc_cdc('H');      //Test Point
            break;

        case 0x4C:              // Caracter 'L' ou 0x4C - Coloca todos os motores em nível baixo;
            inicia_motores(0,64,3891);
            putc_cdc('L');      //Test Point
            break;

        default:                // Não faz nada e retorna para receber proximo controle;
            putc_cdc('N');
            putc_cdc('o');
            putc_cdc('p');

    }
    return 1;
}


/*Função de recebimento e aplicação de dados.
 Dependendo do controle selecionado, colocará um valor padrão de dados ou
 receberá os dados da Rasp. Posteriormente aplicará os 60 valores recebidos*/
int processa_dado( char dado )
{
    char data[10];                          // Declara o buffer de dados para 60 motores;
    char confirmacao;                       // Buffer que receberá a indicaçao de final de transmissão;
    unsigned short indice = 0;              // Declara uma variavel para auxiliar no recebimento do buffer (Vetor)
    if(dado == '_')                         // Testes de motor com valores pré-estabelecidos
    {
        data[0]='?';                        //? = 0b00111111 -> Motores terão os valores 111 111
        data[1]='0';                        //0 = 0b00110000 -> Motores terão os valores 110 000
        data[2]='0';
        data[3]='0';
        data[4]='0';
        data[5]='0';
        data[6]='0';
        data[7]='0';
        data[8]='0';
        data[9]='?';
        ativa_dados(data);                  // Ativa os dados predeterminados;
        putc_cdc('K');                      // Test Point
    }
    else if(dado == '@')                    // Recebimento e aplicação de dados;
    {
        putc_cdc('D');                      // Test Ponit
        while(indice<10)                    // Processo de recebimento de 10 bytes da Rasp;
        {
            data[indice]=recebe_dado_usb(); // Chama rotina de recebimento de um byte e o armazena;
            putc_cdc('-');                  // Test Point
            indice++;                       // Incrementa o vetor;
        }
        putc_cdc('Y');                      // Test Ponit
        confirmacao=recebe_dado_usb();      // Chama rotina de recebimentode um byte e o armazena;
        /*Verifica se o byte recebido por último é igual ao byte de finalização
         * de transmissão; */
        if(confirmacao!='A')                // 0x41 - Caracter 'A' no qual define o final da transmissão;
        {
            putc_cdc('E');                  //Erro de transmissão - Sinaliza a rasp que houve um erro;
        }
        else
        {
            ativa_dados(data);              // Ativa os dados recebidos;
            putc_cdc('K');                  // Traminssão OK - Sinaliza a rasp que a transmissão foi um sucesso;
        }
    }
    return 1;                               //Retorna para aguardar os proximos comandos;
}


/*Função de inicialização dos motore, no qual pode ser usada para colocar todos
 * os motoroes em High ou Low ;*/
void inicia_motores( unsigned char inicial, unsigned char final, int quanto )
{
    unsigned char canal;
    for(canal = inicial; canal < final; canal++)    // Envia para todos os canais, determinado nivel de PWM
    {
        setGrayScaleValue(canal, quanto);           // Define determinado PWM para determinado Canal
    }
    updateTlc5940();                                // Atualiza todos os TLCs
}


/*Configuração inicial do Pic*/
void config_pic(void)
{
    /* Configura o port RA1/AN1 como port de entrada analógico
     * As referências de tensão usadas são as internas do PIC (0V e 5V)
     * Este port será usado para verificação de expansão, no qual
     será feito por um divisor de tensão que indicará qual expansão
     esta conectada;*/
    OpenADC(ADC_FOSC_8 & ADC_RIGHT_JUST & ADC_20_TAD, ADC_CH1 & ADC_INT_OFF & ADC_VREFPLUS_VDD & ADC_VREFMINUS_VSS, ADC_2ANA);
    
    TRISA = 0x0F;       // PortA 0 a 3 configurados como entrada (analógica) e 4 a 7 como saída;
    TRISB = 0x00;       // PortB 0 a 7 como saída;
    TRISC = 0x00;       // PortC 0 a 7 como saída;
    TRISD = 0x00;       // PortD 0 a 7 como saída;
    TRISE = 0x00;       // PortE 0 a 7 como saída;
    PORTA = 0x00;       // Todo portA com nível lógico baixo;
    PORTB = 0x00;       // Todo portB com nível lógico baixo;
    PORTC = 0x00;       // Todo portC com nível lógico baixo;
    PORTD = 0x00;       // Todo portD com nível lógico baixo;
    PORTE = 0x00;       // Todo portE com nível lógico baixo;

    ADCON1 = 0x0D;      // 0b0000 1011 - Port A0 até A3 como analógico (Bit 3,2,1 e 0) Setado como +5 e 0V (Bit 6 e 5)
    SSPCON1bits.SSPEN = 1;
}


/* * Função desenvolvida para verificar o nível de tensão no Port AN1, no qual verificará se existe expansão conectada
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
void config_expansao(void)
{
    unsigned short tensao;                  // Armazena o valor da conversão ADC feita;
    unsigned short numeroDeVerificacao = 0; // Sinalização de erro de leitura;

    SetChanADC(ADC_CH1);            // Seta o canal analógico 1 (AN1) no qual verifica a existencia ou não de expansão por um divisor de tensão
    ConvertADC();                   //Inicia conversão ADC
    while(BusyADC());               // Aguarda a finalização da conversão
    tensao = ReadADC();             //Guarda a informação obtida da conversão
    while(1)
    {
        // Sem Expansão (10 Celulas)
        if(tensao > 716)              //Verifica se a tensão da porta AN0 é maior que 3,5V (717/1024*5V=3,501V)
        {
            if(tamanhoDeExpansao == '0')
            {
                return;
            }
            tamanhoDeExpansao = '0';
        }
        // Expansão no tamanho pequeno (10 + 10 Celulas = 20 )
        else if(tensao < 206)         //Verifica se a tensão da porta AN0 é menor que 1V (205/1024*5V=1,001V) (O ideal é que na porta tenha 0V)
        {
            if(tamanhoDeExpansao == '1')
            {
                return;
            }
            tamanhoDeExpansao = '1';
        }
        // Expansão no tamanho máximo (10 + 20 Celulas = 30)
        else if(tensao > 306 && tensao < 615) //Verifica se a tensão da porta AN0 é maior que 1,5V (307/1024*5=1,499V) e menor que 3V (614/1024*5=2,998V))
        {
            if(tamanhoDeExpansao == '2')
            {
                return;
            }
            tamanhoDeExpansao = '2';
        }
        else
        {
            if(numeroDeVerificacao > 1) //Caso o processo faça mais que 3 mediçoes no ADC sem fazer 2 medidas consecutivas iguais, será informado erro
            {
                tamanhoDeExpansao = 'X';
                return;
            }
        }
        numeroDeVerificacao++;
    }
}


/* Esta função é responsavel por acionar cada motor de acordo com os dados recebidos.
 A função receberá um conjunto de 10 Bytes, tendo cada Byte, 6 bits úteis (6 menos significativos).
 Cada Byte com 6 bits úteis é usado para uma celula (6 pinos), sendo cada pino/canal acionado
 diretamente por cada bit.*/
void ativa_dados( char dados[10])
{
    int valor;
    char mascara;                       // Mascara usada para verificar bit a bit se o canal deve ser acionado ou não;
    unsigned short celula = 0;          // Define qual celula esta sendo trabalhada;
    putc_cdc(' ');                      // Test Point;
    while(celula<10)                    // Trabalha uma celula de cada vez (10 celulas);
    {
        mascara = 0b00100000;           // Define mascará p/ um bit, começando do bit 5;
        valor=0;                        // Define qual dos 6 canais da celula esta sendo trabalhado;
        while (mascara != 0b00000000)   // Verdadeira até que os 6 bits tenham sido analisados;
        {
            if((dados[celula] & mascara) == 0b00000000) // Bit analisado for zero;
            {
                setGrayScaleValue(celula*6+valor, PINO_BAIXO); // Aciona em determinada celula, em determinado canal o valor '0';
                putc_cdc('0');          // Test Point;
            }
            else                        // Bit analisado for 1;
            {
                setGrayScaleValue(celula*6+valor, PINO_ALTO); // Aciona em determinada celula, em determinado canal o valor '1';
                putc_cdc('1');          // Test Point;
            }
            mascara = mascara >> 1;     // Rotaciona a mascara p/ o proximo canal/bit;
            valor++;                    // Altera p/ o proximo canal da celula (0 a 5);
        }
        celula++;                       // Altera p/ a proxima celula (0 a 9);
        putc_cdc(' ');                  // Test Point
    }
    updateTlc5940();                    // Atualiza de uma única vez todos os canais nos TLCs;
}