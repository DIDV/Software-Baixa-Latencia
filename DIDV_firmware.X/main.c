#include <p18f4550.h>
#include "tlc5940.h"
#include <timers.h>
#include <adc.h>
#include "usb_functions.h"

//\TODO: modificar para valores reais
#define PINO_ALTO 3791  // Define o valor do PWM para nível lógico alto; //3687
#define PINO_BAIXO 3891 // Define o valor do PWM para nível lógico baixo; //3891

char tamanhoDeExpansao = '-';       // Variavel que armazena o tamanho da expansão;
char data[10];                      // Declara o buffer de dados para 60 motores;
char memory;                        // Buffer para armazenar o dado que irá piscar (função blink_cell);
char ativa_blink = '0';             // Aciona o funcionamento da função blink_cell;
char blink = '0';                   // Indica se a célula que irá piscar esta ou não mostrando o dado inicial;
unsigned short cell_blink;          // Armazena qual das 10 células irá piscar (blink);
long int blink_time = 0;            // Contador para que periodicamente pisque determinada célula;
unsigned short persistencia = 0;        // Contador para manter o led um pouco mais tempo ligado.

//Interrupção de alta prioridade para funcionamento do TLC5940;
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
             PORTDbits.RD2 = 0;         // Desliga LED de indicação de recebimento de controle
             persistencia = 0;          // Reseta contatador;
         }
         persistencia++;                // Incrementa contador;
         INTCONbits.TMR0IF=0;
     }
}

// Declarações das funções;
void processa_controle( char controle );            // Função que toma as decisões de acordo com a solicitação do alto nível;
void processa_dado( char dado );                    // Recebe e processa os DADOS BRAILLE
char recebe_dado_usb();                             // Verifica se algum dado foi recebido do alto nível;
void inicia_motores();                              // Função para colocar todos os motores em nível alto ou baixo;
void config_pic();                                  // Configura os ports, timers, etc.
void config_expansao();                             // Verifica o tamnha da expansão conectada, caso exista uma;
void ativa_dados(char dados[10]);                   // Aciona os motores de acordo com os dados;
void vibracall_motor (unsigned short tempo);        // Função para vibrar um motor, de acordo com o tempo inserido;
void blink_cell(unsigned short cell);

void main(void)
{
    char byte_recebido;
    config_pic();                       // Configurações iniciais do PIC (Ports, etc.);
    initialiseTlc5940();                // Configuração do sistema de acionamento dos drivers;
    config_expansao();                  // Verificação de existencia de expansão;
    usb_install();                      // Inicialização do USB;
    inicia_motores(0,64,PINO_BAIXO);    // Configura os motores inicialmente p/ posição zero;
    Delay_ms(3);                        // Aguarda um tempo antes de acionar o rele de alimentação dos motores
    PORTDbits.RD7 = 1;                  // Bit de acionamento de rele (alimentação dos motores)
    PORTDbits.RD3 = 1;                  // Indica que a placa está ligada.

    do
    {
        byte_recebido = recebe_dado_usb();  // Recebe os dados vindos da rasp;
        processa_controle(byte_recebido);   // Envia o dado obtido para que o controle tome uma decisão;
    }while(1);
}


/* Função no qual tem como objetivo buscar os dados no buffer de entrada do
 PIC, só retornando dela quando alguma informação for recebida*/
char recebe_dado_usb()
{
    char byte_recebido;

    do
    {
        blink_time = blink_time + 1;        // Incrementa contador de blink;
        if(blink_time == 500000)            // Verifica se já ocorreu 500.000 ciclos;
        {
            blink_time = 0;                 // Reseta contador;
            if(ativa_blink == '1')          // Verifica se a função blink_cell esta ativa;
            {
                blink_cell(cell_blink);     // Chama função para piscar determinada célula (cell_blink);
            }
        }
        /*Maquina de estados do USB, no qual verifica se o mesmo esta configurado
        e executa a inclusão e exclusão de dados nos buffer de entrada e saída.*/
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




/* Função de controle, no qual toma as decisões do que será feito de acordo
 com o comando recebido da Rasp;*/
void processa_controle( char controle )
{
    switch(controle)                // Seleção do comando
    {
        case 0x40:                  // Caracter '@' ou 0x40 - Processo de recebimento de dados;
            processa_dado('@');     // Chama função processa_dado com objetivo de transmitir o texto que será exibido na linha;
            break;

        case 0x2B:                  // Caracter '+' ou 0x2B - Responde com o último valor que foi aplicado na linha;
            processa_dado('+');     // Chama função processa_dado com objetivo de informar o alto nivel qual foi o útimo dado transmitido;
            break;

        case 0x5F:                  // Caracter '_' ou 0x5F - Processo de dados teste pré-estabelecidos (Comando de Teste);
            processa_dado('_');     // Chama função processa_dado com objetivo de acionar dados pré-estabelecidos;
            break;

        case 0x51:                  // Caracter 'Q' ou 0x51 - Processo de verificação do tamanho da expansão;
            putc_cdc(tamanhoDeExpansao);    // Responde com o tamanho atual da expansão (conforme ultima analise feita);
            /* O comando putc_cdc envia um byte (Char) para o buffer de saida do PIC, de modo que o alto nível possa receber esta resposta;  */
            break;

        case 0x45:                  // Caracter 'E' ou 0x45 - Processo de reconfiguração de expansão;
            config_expansao();      // Chamaa função de reconfiguração de expansão;
            putc_cdc('K');          // Confirma ao alto nível que a verificação de expansão foi refeita;
            break;

        case 0x48:                  // Caracter 'H' ou 0x48 - Coloca todos os motores em nível alto;
            inicia_motores(0,64,PINO_ALTO);  //Aciona todos os motores em determinado PWM;
            putc_cdc('K');          //Confirma ao alto nível que os motores foram acionados;
            break;

        case 0x4C:                  // Caracter 'L' ou 0x4C - Coloca todos os motores em nível baixo;
            inicia_motores(0,64,PINO_BAIXO);  //Aciona todos os motores em determinado PWM;
            putc_cdc('K');          //Confirma ao alto nível que os motores foram acionados;
            break;

        //Comandos para piscar determinada célula, para que o deficiente possa identificar qual célula o ponteiro esta;
        case 0xB0:                  // Comando de acionamento da função blink, para piscar a célula 0;
            cell_blink = 0;         // Seta célula 0 para piscar;
            ativa_blink = '1';      // Aciona o sistema de piscar (blink);
            break;
        case 0xB1:                  // Comando de acionamento da função blink, para piscar a célula 1;
            cell_blink = 1;         // Seta célula 1 para piscar;
            ativa_blink = '1';      // Aciona o sistema de piscar (blink);
            break;
        case 0xB2:                  // Comando de acionamento da função blink, para piscar a célula 2;
            cell_blink = 2;         // Seta célula 2 para piscar;
            ativa_blink = '1';      // Aciona o sistema de piscar (blink);
            break;
        case 0xB3:                  // Comando de acionamento da função blink, para piscar a célula 3;
            cell_blink = 3;         // Seta célula 3 para piscar;
            ativa_blink = '1';      // Aciona o sistema de piscar (blink);
            break;
        case 0xB4:                  // Comando de acionamento da função blink, para piscar a célula 4;
            cell_blink = 4;         // Seta célula 4 para piscar;
            ativa_blink = '1';      // Aciona o sistema de piscar (blink);
            break;
        case 0xB5:                  // Comando de acionamento da função blink, para piscar a célula 5;
            cell_blink = 5;         // Seta célula 5 para piscar;
            ativa_blink = '1';      // Aciona o sistema de piscar (blink);
            break;
        case 0xB6:                  // Comando de acionamento da função blink, para piscar a célula 6;
            cell_blink = 6;         // Seta célula 6 para piscar;
            ativa_blink = '1';      // Aciona o sistema de piscar (blink);
            break;
        case 0xB7:                  // Comando de acionamento da função blink, para piscar a célula 7;
            cell_blink = 7;         // Seta célula 7 para piscar;
            ativa_blink = '1';      // Aciona o sistema de piscar (blink);
            break;
        case 0xB8:                  // Comando de acionamento da função blink, para piscar a célula 8;
            cell_blink = 8;         // Seta célula 8 para piscar;
            ativa_blink = '1';      // Aciona o sistema de piscar (blink);
            break;
        case 0xB9:                  // Comando de acionamento da função blink, para piscar a célula 9;
            cell_blink = 9;         // Seta célula 9 para piscar;
            ativa_blink = '1';      // Aciona o sistema de piscar (blink);
            break;
        case 0xC0:
            ativa_blink = '0';          //  Desliga o sistema de piscar (blink);
            data[cell_blink] = memory;  // Retorna o caracter original no lugar da célula em branco;
            ativa_dados(data);          // Ativa a nova informação nos motores;
            blink = '0';                // Reseta o estado de blink;
            break;

        case 0x21:                  //Caracter '!' ou 0x21 - Aciona o motor pelo tempo de 5 segundo;
            vibracall_motor(5000);  //Aciona o vibracall por 5 Segundos (tempo máximo permitido);
            break;

        default:                    // Não faz nada e retorna para receber proximo controle;
            putc_cdc('N');          // Indica ao alto nível que o controle solicitado não existe;

    }
    return;
}


/*Função de recebimento e aplicação de dados.
 Dependendo do controle selecionado, colocará um valor padrão de dados ou
 receberá os dados da Rasp. Posteriormente aplicará os 60 valores recebidos.
 Também permite transmitir o último dado recebido ao alto nível;*/
void processa_dado( char controle )
{
    char confirmacao;                       // Buffer que receberá a indicaçao de final de transmissão;
    unsigned short indice = 0;              // Declara uma variavel para auxiliar no recebimento do buffer (Vetor)
    if((ativa_blink == '1') && (controle != '+'))   // Caso seja enviado uma nova trasmissão de dados
    {
        ativa_blink = '0';          // Desliga o sistema de piscar (blink);
        blink = '0';                // Reseta o estado de blink
    }
    if(controle == '_')                     // Testes de motor com valores pré-estabelecidos
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
        putc_cdc('K');                      //Confirma ao alto nível que os motores foram acionados;
    }
    else if(controle == '@')                // Recebimento e aplicação de dados;
    {
        while(indice<10)                    // Processo de recebimento de 10 bytes da Rasp;
        {
            data[indice]=recebe_dado_usb(); // Chama rotina de recebimento de um byte e o armazena;
            indice++;                       // Incrementa o vetor;
        }
        confirmacao=recebe_dado_usb();      // Chama rotina de recebimentode um byte e o armazena;
        /*Verifica se o byte recebido por último é igual ao byte de finalização de transmissão; */
        if(confirmacao!='A')                // 0x41 - Caracter 'A' no qual define o final da transmissão;
        {
            putc_cdc('E');                  //Erro de transmissão - Sinaliza a rasp que houve um erro;
        }
        else
        {
            ativa_dados(data);              // Ativa os dados recebidos;
            vibracall_motor(100);           // Vibra o motor por 100 ms;
            putc_cdc('K');                  // Traminssão OK - Sinaliza a rasp que a transmissão foi um sucesso;
        }
    }
    else if(controle == '+')                // Responde o último dado recebido;
    {
        while(indice<10)                    // Processo de transmissão de 10 bytes para Rasp;
        {
            putc_cdc(data[indice]);         // Transmite o byte da posição indice;
            indice++;                       // Incrementa posição indice;
        }
        putc_cdc('K');                      // Informa final de transmissão;
    }
    return;                               //Retorna para aguardar os proximos comandos;
}


/*Função de inicialização dos motore, no qual pode ser usada para colocar todos
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


/*Configuração inicial do Pic*/
void config_pic(void)
{
    /* Configura o port AN0, 1, 2 e 3 como port de entrada analógico
     * As referências de tensão usadas são as internas do PIC (0V e 5V)
     * Este port será usado para verificação de expansão, no qual
     será feito por um divisor de tensão que indicará qual expansão
     esta conectada;*/
    OpenADC(ADC_FOSC_8 & ADC_RIGHT_JUST & ADC_20_TAD, ADC_CH1 & ADC_INT_OFF & ADC_VREFPLUS_VDD & ADC_VREFMINUS_VSS, ADC_4ANA);
    
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
    while(celula<10)                    // Trabalha uma celula de cada vez (10 celulas);
    {
        mascara = 0b00100000;           // Define mascará p/ um bit, começando do bit 5;
        valor=0;                        // Define qual dos 6 canais da celula esta sendo trabalhado;
        while (mascara != 0b00000000)   // Verdadeira até que os 6 bits tenham sido analisados;
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
    updateTlc5940();                    // Atualiza de uma única vez todos os canais nos TLCs;
}

/* Vibra um motor com tempos multiplos de 1mSeg;
/ Chamar a função de acordo com quantas vezes gostaria de esperar 1 mSeg;               */
void vibracall_motor (unsigned short tempo)
{
    if(tempo > 5000)            // Impede que coloquem tempos maiores que 5 segundos, pois paraliza todo o hardware durante esse período;
    {
        tempo = 5000;
    }
    PORTDbits.RD5 = 1;          // Aciona saída do vibracall
    Delay_ms(tempo);            // Aguarda Tempo mSeg
    PORTDbits.RD5 = 0;          // Desliga saída do vibracall
}

/*Função para piscar determinada célula do display braille;
 Com essa função, o deficiente visual consegue descobrir qual célula
 esta o ponteiro de escrita nos momentos de escrita/edição de texto; */
void blink_cell(unsigned short cell)    // A variável cell indica qual das células irá piscar (0 até 9);
{
    char clear = 0x00;                  // A variável clear é um caracter em branco, no qual irá aparecer na célula que pisca;
    if(data[cell] != clear)             // Se o dado na célula que irá piscar for diferente de zero, armazena em um buffer esse dado;
    {
        memory = data[cell];            // Armazenagem do dado que não deve ser perdido em um buffer alternativo;
    }
    if(blink == '0')                    // Verifica se determinada célula esta em branco ou com seu valor normal;
    {
        data[cell] = clear;             // Coloca um espaço em branco em determinada célula;
        ativa_dados(data);              // Habilita o dado, alterando determinada célula por espaço em branco;
        blink = '1';                    // Indica que determinada célula agora esta em branco;
    }
    else                                // Caso a célula já esteja em branco, retorna ao valor anterior;
    {
        data[cell] = memory;            // Retorna o caracter original no lugar da célula em branco;
        ativa_dados(data);              // Habilita os dados originais;
        blink = '0';                    // Indica que determinada célula agora esta com o dado original;
    }
}