#include <p18f4550.h>
#include "tlc5940.h"
#include <timers.h>
#include <adc.h>
#include "usb_functions.h"

//\TODO: modificar para valores reais
#define PINO_ALTO 3791  // Define o valor do PWM para n�vel l�gico alto; //3687
#define PINO_BAIXO 3891 // Define o valor do PWM para n�vel l�gico baixo; //3891

char tamanhoDeExpansao = '-';       // Variavel que armazena o tamanho da expans�o;
char data[10];                      // Declara o buffer de dados para 60 motores;
char memory;                        // Buffer para armazenar o dado que ir� piscar (fun��o blink_cell);
char ativa_blink = '0';             // Aciona o funcionamento da fun��o blink_cell;
char blink = '0';                   // Indica se a c�lula que ir� piscar esta ou n�o mostrando o dado inicial;
unsigned short cell_blink;          // Armazena qual das 10 c�lulas ir� piscar (blink);
long int blink_time = 0;            // Contador para que periodicamente pisque determinada c�lula;
unsigned short persistencia = 0;        // Contador para manter o led um pouco mais tempo ligado.

//Interrup��o de alta prioridade para funcionamento do TLC5940;
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
         WriteTimer0(XLATCOUNTTIMER);
         processXLATinterrupt();
         if(persistencia == 10)         // Mantem o LED que indica recebimento de controle ligado por um tempo maior;
         {
             PORTDbits.RD2 = 0;         // Desliga LED de indica��o de recebimento de controle
             persistencia = 0;          // Reseta contatador;
         }
         persistencia++;                // Incrementa contador;
         INTCONbits.TMR0IF=0;
     }
}

// Declara��es das fun��es;
void processa_controle( char controle );            // Fun��o que toma as decis�es de acordo com a solicita��o do alto n�vel;
void processa_dado( char dado );                    // Recebe e processa os DADOS BRAILLE
char recebe_dado_usb();                             // Verifica se algum dado foi recebido do alto n�vel;
void inicia_motores();                              // Fun��o para colocar todos os motores em n�vel alto ou baixo;
void config_pic();                                  // Configura os ports, timers, etc.
void config_expansao();                             // Verifica o tamnha da expans�o conectada, caso exista uma;
void ativa_dados(char dados[10]);                   // Aciona os motores de acordo com os dados;
void vibracall_motor (unsigned short tempo);        // Fun��o para vibrar um motor, de acordo com o tempo inserido;
void blink_cell(unsigned short cell);

void main(void)
{
    char byte_recebido;
    config_pic();                       // Configura��es iniciais do PIC (Ports, etc.);
    initialiseTlc5940();                // Configura��o do sistema de acionamento dos drivers;
    config_expansao();                  // Verifica��o de existencia de expans�o;
    usb_install();                      // Inicializa��o do USB;
    inicia_motores(0,64,PINO_BAIXO);    // Configura os motores inicialmente p/ posi��o zero;
    Delay_ms(3);                        // Aguarda um tempo antes de acionar o rele de alimenta��o dos motores
    PORTDbits.RD7 = 1;                  // Bit de acionamento de rele (alimenta��o dos motores)
    PORTDbits.RD3 = 1;                  // Indica que a placa est� ligada.

    do
    {
        byte_recebido = recebe_dado_usb();  // Recebe os dados vindos da rasp;
        processa_controle(byte_recebido);   // Envia o dado obtido para que o controle tome uma decis�o;
    }while(1);
}


/* Fun��o no qual tem como objetivo buscar os dados no buffer de entrada do
 PIC, s� retornando dela quando alguma informa��o for recebida*/
char recebe_dado_usb()
{
    char byte_recebido;

    do
    {
        blink_time = blink_time + 1;        // Incrementa contador de blink;
        if(blink_time == 500000)            // Verifica se j� ocorreu 500.000 ciclos;
        {
            blink_time = 0;                 // Reseta contador;
            if(ativa_blink == '1')          // Verifica se a fun��o blink_cell esta ativa;
            {
                blink_cell(cell_blink);     // Chama fun��o para piscar determinada c�lula (cell_blink);
            }
        }
        /*Maquina de estados do USB, no qual verifica se o mesmo esta configurado
        e executa a inclus�o e exclus�o de dados nos buffer de entrada e sa�da.*/
        usb_handler();
        
        /* O valor 0xFF funciona como padrao para a variavel byte_recebido,
        e significa que nenhum byte foi recebido pelo PIC. */
        byte_recebido = 0xFF;

        /* Caso tenha algum char no buffer de entrada do PIC,
        esse sera' transferido para a variavel byte_recebido.
        Caso nao, o valor inicial 0xFF sera' mantido. */
        poll_getc_cdc(&byte_recebido);

        /* Se nenhum byte tiver sido recebido, esse ciclo do loop
        sera' encerrado nesse ponto. */
    } while ( byte_recebido == 0xFF );

    PORTDbits.RD2 = 1;                      // Aciona um LED indicando que um controle foi recebido;
    return byte_recebido;                   // Retorna o dado recebido;

} 




/* Fun��o de controle, no qual toma as decis�es do que ser� feito de acordo
 com o comando recebido da Rasp;*/
void processa_controle( char controle )
{
    switch(controle)                // Sele��o do comando
    {
        case 0x40:                  // Caracter '@' ou 0x40 - Processo de recebimento de dados;
            processa_dado('@');     // Chama fun��o processa_dado com objetivo de transmitir o texto que ser� exibido na linha;
            break;

        case 0x2B:                  // Caracter '+' ou 0x2B - Responde com o �ltimo valor que foi aplicado na linha;
            processa_dado('+');     // Chama fun��o processa_dado com objetivo de informar o alto nivel qual foi o �timo dado transmitido;
            break;

        case 0x5F:                  // Caracter '_' ou 0x5F - Processo de dados teste pr�-estabelecidos (Comando de Teste);
            processa_dado('_');     // Chama fun��o processa_dado com objetivo de acionar dados pr�-estabelecidos;
            break;

        case 0x51:                  // Caracter 'Q' ou 0x51 - Processo de verifica��o do tamanho da expans�o;
            putc_cdc(tamanhoDeExpansao);    // Responde com o tamanho atual da expans�o (conforme ultima analise feita);
            /* O comando putc_cdc envia um byte (Char) para o buffer de saida do PIC, de modo que o alto n�vel possa receber esta resposta;  */
            break;

        case 0x45:                  // Caracter 'E' ou 0x45 - Processo de reconfigura��o de expans�o;
            config_expansao();      // Chamaa fun��o de reconfigura��o de expans�o;
            putc_cdc('K');          // Confirma ao alto n�vel que a verifica��o de expans�o foi refeita;
            break;

        case 0x48:                  // Caracter 'H' ou 0x48 - Coloca todos os motores em n�vel alto;
            inicia_motores(0,64,PINO_ALTO);  //Aciona todos os motores em determinado PWM;
            putc_cdc('K');          //Confirma ao alto n�vel que os motores foram acionados;
            break;

        case 0x4C:                  // Caracter 'L' ou 0x4C - Coloca todos os motores em n�vel baixo;
            inicia_motores(0,64,PINO_BAIXO);  //Aciona todos os motores em determinado PWM;
            putc_cdc('K');          //Confirma ao alto n�vel que os motores foram acionados;
            break;

        //Comandos para piscar determinada c�lula, para que o deficiente possa identificar qual c�lula o ponteiro esta;
        case 0xB0:                  // Comando de acionamento da fun��o blink, para piscar a c�lula 0;
            cell_blink = 0;         // Seta c�lula 0 para piscar;
            ativa_blink = '1';      // Aciona o sistema de piscar (blink);
            break;
        case 0xB1:                  // Comando de acionamento da fun��o blink, para piscar a c�lula 1;
            cell_blink = 1;         // Seta c�lula 1 para piscar;
            ativa_blink = '1';      // Aciona o sistema de piscar (blink);
            break;
        case 0xB2:                  // Comando de acionamento da fun��o blink, para piscar a c�lula 2;
            cell_blink = 2;         // Seta c�lula 2 para piscar;
            ativa_blink = '1';      // Aciona o sistema de piscar (blink);
            break;
        case 0xB3:                  // Comando de acionamento da fun��o blink, para piscar a c�lula 3;
            cell_blink = 3;         // Seta c�lula 3 para piscar;
            ativa_blink = '1';      // Aciona o sistema de piscar (blink);
            break;
        case 0xB4:                  // Comando de acionamento da fun��o blink, para piscar a c�lula 4;
            cell_blink = 4;         // Seta c�lula 4 para piscar;
            ativa_blink = '1';      // Aciona o sistema de piscar (blink);
            break;
        case 0xB5:                  // Comando de acionamento da fun��o blink, para piscar a c�lula 5;
            cell_blink = 5;         // Seta c�lula 5 para piscar;
            ativa_blink = '1';      // Aciona o sistema de piscar (blink);
            break;
        case 0xB6:                  // Comando de acionamento da fun��o blink, para piscar a c�lula 6;
            cell_blink = 6;         // Seta c�lula 6 para piscar;
            ativa_blink = '1';      // Aciona o sistema de piscar (blink);
            break;
        case 0xB7:                  // Comando de acionamento da fun��o blink, para piscar a c�lula 7;
            cell_blink = 7;         // Seta c�lula 7 para piscar;
            ativa_blink = '1';      // Aciona o sistema de piscar (blink);
            break;
        case 0xB8:                  // Comando de acionamento da fun��o blink, para piscar a c�lula 8;
            cell_blink = 8;         // Seta c�lula 8 para piscar;
            ativa_blink = '1';      // Aciona o sistema de piscar (blink);
            break;
        case 0xB9:                  // Comando de acionamento da fun��o blink, para piscar a c�lula 9;
            cell_blink = 9;         // Seta c�lula 9 para piscar;
            ativa_blink = '1';      // Aciona o sistema de piscar (blink);
            break;
        case 0xC0:
            ativa_blink = '0';          //  Desliga o sistema de piscar (blink);
            data[cell_blink] = memory;  // Retorna o caracter original no lugar da c�lula em branco;
            ativa_dados(data);          // Ativa a nova informa��o nos motores;
            blink = '0';                // Reseta o estado de blink;
            break;

        case 0x21:                  //Caracter '!' ou 0x21 - Aciona o motor pelo tempo de 5 segundo;
            vibracall_motor(5000);  //Aciona o vibracall por 5 Segundos (tempo m�ximo permitido);
            break;

        default:                    // N�o faz nada e retorna para receber proximo controle;
            putc_cdc('N');          // Indica ao alto n�vel que o controle solicitado n�o existe;

    }
    return;
}


/*Fun��o de recebimento e aplica��o de dados.
 Dependendo do controle selecionado, colocar� um valor padr�o de dados ou
 receber� os dados da Rasp. Posteriormente aplicar� os 60 valores recebidos.
 Tamb�m permite transmitir o �ltimo dado recebido ao alto n�vel;*/
void processa_dado( char controle )
{
    char confirmacao;                       // Buffer que receber� a indica�ao de final de transmiss�o;
    unsigned short indice = 0;              // Declara uma variavel para auxiliar no recebimento do buffer (Vetor)
    if((ativa_blink == '1') && (controle != '+'))   // Caso seja enviado uma nova trasmiss�o de dados
    {
        ativa_blink = '0';          // Desliga o sistema de piscar (blink);
        blink = '0';                // Reseta o estado de blink
    }
    if(controle == '_')                     // Testes de motor com valores pr�-estabelecidos
    {
        data[0]='?';                        //? = 0b00111111 -> Motores ter�o os valores 111 111
        data[1]='0';                        //0 = 0b00110000 -> Motores ter�o os valores 110 000
        data[2]='0';
        data[3]='0';
        data[4]='0';
        data[5]='0';
        data[6]='0';
        data[7]='0';
        data[8]='0';
        data[9]='?';
        ativa_dados(data);                  // Ativa os dados predeterminados;
        putc_cdc('K');                      //Confirma ao alto n�vel que os motores foram acionados;
    }
    else if(controle == '@')                // Recebimento e aplica��o de dados;
    {
        while(indice<10)                    // Processo de recebimento de 10 bytes da Rasp;
        {
            data[indice]=recebe_dado_usb(); // Chama rotina de recebimento de um byte e o armazena;
            indice++;                       // Incrementa o vetor;
        }
        confirmacao=recebe_dado_usb();      // Chama rotina de recebimentode um byte e o armazena;
        /*Verifica se o byte recebido por �ltimo � igual ao byte de finaliza��o de transmiss�o; */
        if(confirmacao!='A')                // 0x41 - Caracter 'A' no qual define o final da transmiss�o;
        {
            putc_cdc('E');                  //Erro de transmiss�o - Sinaliza a rasp que houve um erro;
        }
        else
        {
            ativa_dados(data);              // Ativa os dados recebidos;
            vibracall_motor(100);           // Vibra o motor por 100 ms;
            putc_cdc('K');                  // Traminss�o OK - Sinaliza a rasp que a transmiss�o foi um sucesso;
        }
    }
    else if(controle == '+')                // Responde o �ltimo dado recebido;
    {
        while(indice<10)                    // Processo de transmiss�o de 10 bytes para Rasp;
        {
            putc_cdc(data[indice]);         // Transmite o byte da posi��o indice;
            indice++;                       // Incrementa posi��o indice;
        }
        putc_cdc('K');                      // Informa final de transmiss�o;
    }
    return;                               //Retorna para aguardar os proximos comandos;
}


/*Fun��o de inicializa��o dos motore, no qual pode ser usada para colocar todos
 os motoroes em High ou Low ;*/
void inicia_motores( unsigned char inicial, unsigned char final, int quanto )
{
    unsigned char canal;
    for(canal = inicial; canal < final; canal++)    // Envia para todos os canais, determinado nivel de PWM
    {
        setGrayScaleValue(canal, quanto);           // Define determinado PWM para determinado Canal
    }
    updateTlc5940();                                // Atualiza todos os TLCs
}


/*Configura��o inicial do Pic*/
void config_pic(void)
{
    /* Configura o port AN0, 1, 2 e 3 como port de entrada anal�gico
     * As refer�ncias de tens�o usadas s�o as internas do PIC (0V e 5V)
     * Este port ser� usado para verifica��o de expans�o, no qual
     ser� feito por um divisor de tens�o que indicar� qual expans�o
     esta conectada;*/
    OpenADC(ADC_FOSC_8 & ADC_RIGHT_JUST & ADC_20_TAD, ADC_CH1 & ADC_INT_OFF & ADC_VREFPLUS_VDD & ADC_VREFMINUS_VSS, ADC_4ANA);
    
    TRISA = 0x0F;       // PortA 0 a 3 configurados como entrada (anal�gica) e 4 a 7 como sa�da;
    TRISB = 0x00;       // PortB 0 a 7 como sa�da;
    TRISC = 0x00;       // PortC 0 a 7 como sa�da;
    TRISD = 0x00;       // PortD 0 a 7 como sa�da;
    TRISE = 0x00;       // PortE 0 a 7 como sa�da;
    PORTA = 0x00;       // Todo portA com n�vel l�gico baixo;
    PORTB = 0x00;       // Todo portB com n�vel l�gico baixo;
    PORTC = 0x00;       // Todo portC com n�vel l�gico baixo;
    PORTD = 0x00;       // Todo portD com n�vel l�gico baixo;
    PORTE = 0x00;       // Todo portE com n�vel l�gico baixo;

    SSPCON1bits.SSPEN = 1;
}


/* * Fun��o desenvolvida para verificar o n�vel de tens�o no Port AN1, no qual verificar� se existe expans�o conectada
         e o tamanho da mesma, de acordo com os seguintes parametros:

          -> Sem Expans�o:
          N�vel de tens�o superiores a 3,5V. Idealmente deve estar em 4,55V (Conforme divisor de tens�o utilizado)

          -> Expans�o de 10 Celulas:
          Nivel de tens�o inferiores a 1,0V. Idealmente deve estar em 0V (Conforme divisor de tens�o utilizado)

          -> Expans�o de 20 Celulas:
          N�vel de tens�o na faixa de 1,5 a 3,0V. Idealmente deve estar em 2,3V (Conforme divisor de tens�o utilizado)

          -> Regi�o de incerteza/Erro de leitura:
          N�vel de tens�o nas faixas de 1,0 a 1,5V e 3,0 a 3,5V;
          Para essa situa��o, o software ir� reverificar o n�vel de tens�o novamente, caso caia na regi�o de incerteza por
          tr�s vezes seguidas, ir� selecionar o m�dulo Sem Expans�o e informar o erro ao Alto N�vel;

*/
void config_expansao(void)
{
    unsigned short tensao;                  // Armazena o valor da convers�o ADC feita;
    unsigned short numeroDeVerificacao = 0; // Sinaliza��o de erro de leitura;

    SetChanADC(ADC_CH1);            // Seta o canal anal�gico 1 (AN1) no qual verifica a existencia ou n�o de expans�o por um divisor de tens�o
    ConvertADC();                   //Inicia convers�o ADC
    while(BusyADC());               // Aguarda a finaliza��o da convers�o
    tensao = ReadADC();             //Guarda a informa��o obtida da convers�o
    while(1)
    {
        // Sem Expans�o (10 Celulas)
        if(tensao > 716)              //Verifica se a tens�o da porta AN0 � maior que 3,5V (717/1024*5V=3,501V)
        {
            if(tamanhoDeExpansao == '0')
            {
                return;
            }
            tamanhoDeExpansao = '0';
        }
        // Expans�o no tamanho pequeno (10 + 10 Celulas = 20 )
        else if(tensao < 206)         //Verifica se a tens�o da porta AN0 � menor que 1V (205/1024*5V=1,001V) (O ideal � que na porta tenha 0V)
        {
            if(tamanhoDeExpansao == '1')
            {
                return;
            }
            tamanhoDeExpansao = '1';
        }
        // Expans�o no tamanho m�ximo (10 + 20 Celulas = 30)
        else if(tensao > 306 && tensao < 615) //Verifica se a tens�o da porta AN0 � maior que 1,5V (307/1024*5=1,499V) e menor que 3V (614/1024*5=2,998V))
        {
            if(tamanhoDeExpansao == '2')
            {
                return;
            }
            tamanhoDeExpansao = '2';
        }
        else
        {
            if(numeroDeVerificacao > 1) //Caso o processo fa�a mais que 3 medi�oes no ADC sem fazer 2 medidas consecutivas iguais, ser� informado erro
            {
                tamanhoDeExpansao = 'X';
                return;
            }
        }
        numeroDeVerificacao++;
    }
}


/* Esta fun��o � responsavel por acionar cada motor de acordo com os dados recebidos.
 A fun��o receber� um conjunto de 10 Bytes, tendo cada Byte, 6 bits �teis (6 menos significativos).
 Cada Byte com 6 bits �teis � usado para uma celula (6 pinos), sendo cada pino/canal acionado
 diretamente por cada bit.*/
void ativa_dados( char dados[10])
{
    int valor;
    char mascara;                       // Mascara usada para verificar bit a bit se o canal deve ser acionado ou n�o;
    unsigned short celula = 0;          // Define qual celula esta sendo trabalhada;
    while(celula<10)                    // Trabalha uma celula de cada vez (10 celulas);
    {
        mascara = 0b00100000;           // Define mascar� p/ um bit, come�ando do bit 5;
        valor=0;                        // Define qual dos 6 canais da celula esta sendo trabalhado;
        while (mascara != 0b00000000)   // Verdadeira at� que os 6 bits tenham sido analisados;
        {
            if((dados[celula] & mascara) == 0b00000000) // Bit analisado for zero;
            {
                setGrayScaleValue(celula*6+valor, PINO_BAIXO); // Aciona em determinada celula, em determinado canal o valor '0';
            }
            else                        // Bit analisado for 1;
            {
                setGrayScaleValue(celula*6+valor, PINO_ALTO); // Aciona em determinada celula, em determinado canal o valor '1';
            }
            mascara = mascara >> 1;     // Rotaciona a mascara p/ o proximo canal/bit;
            valor++;                    // Altera p/ o proximo canal da celula (0 a 5);
        }
        celula++;                       // Altera p/ a proxima celula (0 a 9);
    }
    updateTlc5940();                    // Atualiza de uma �nica vez todos os canais nos TLCs;
}

/* Vibra um motor com tempos multiplos de 1mSeg;
/ Chamar a fun��o de acordo com quantas vezes gostaria de esperar 1 mSeg;               */
void vibracall_motor (unsigned short tempo)
{
    if(tempo > 5000)            // Impede que coloquem tempos maiores que 5 segundos, pois paraliza todo o hardware durante esse per�odo;
    {
        tempo = 5000;
    }
    PORTDbits.RD5 = 1;          // Aciona sa�da do vibracall
    Delay_ms(tempo);            // Aguarda Tempo mSeg
    PORTDbits.RD5 = 0;          // Desliga sa�da do vibracall
}

/*Fun��o para piscar determinada c�lula do display braille;
 Com essa fun��o, o deficiente visual consegue descobrir qual c�lula
 esta o ponteiro de escrita nos momentos de escrita/edi��o de texto; */
void blink_cell(unsigned short cell)    // A vari�vel cell indica qual das c�lulas ir� piscar (0 at� 9);
{
    char clear = 0x00;                  // A vari�vel clear � um caracter em branco, no qual ir� aparecer na c�lula que pisca;
    if(data[cell] != clear)             // Se o dado na c�lula que ir� piscar for diferente de zero, armazena em um buffer esse dado;
    {
        memory = data[cell];            // Armazenagem do dado que n�o deve ser perdido em um buffer alternativo;
    }
    if(blink == '0')                    // Verifica se determinada c�lula esta em branco ou com seu valor normal;
    {
        data[cell] = clear;             // Coloca um espa�o em branco em determinada c�lula;
        ativa_dados(data);              // Habilita o dado, alterando determinada c�lula por espa�o em branco;
        blink = '1';                    // Indica que determinada c�lula agora esta em branco;
    }
    else                                // Caso a c�lula j� esteja em branco, retorna ao valor anterior;
    {
        data[cell] = memory;            // Retorna o caracter original no lugar da c�lula em branco;
        ativa_dados(data);              // Habilita os dados originais;
        blink = '0';                    // Indica que determinada c�lula agora esta com o dado original;
    }
}