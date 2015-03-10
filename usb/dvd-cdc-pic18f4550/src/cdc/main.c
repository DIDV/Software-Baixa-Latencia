#include "usb_functions.h"

/* TAREFAS
 *
 * 1. criar uma funcao main simples e legivel.
 * 
 */

int processa_controle( char controle );
int processa_dado( char dado );

void main(void)
{
    char byte_recebido;
    char mascara_tipo_byte = 0xC0;
    char mascara_valor_byte = 0x3F;
    char tipo_byte;
    char valor_byte;

    usb_install();

    do
    {
        /* O valor 0xFF funciona como padrao para a variavel byte_recebido,
         * e significa que nenhum byte foi recebido pelo PIC. */
        byte_recebido = 0xFF;

        /* Caso tenha algum char no buffer de entrada do PIC,
         * esse sera' transferido para a variavel byte_recebido.
         * Caso nao, o valor inicial 0xFF sera' mantido. */
        poll_getc_cdc(&byte_recebido);

        /* Se nenhum byte tiver sido recebido, esse ciclo do loop
         * sera' encerrado nesse ponto. */
        if ( byte_recebido == 0xFF ) continue;

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
            processa_controle(valor_byte);
        else
            processa_dado(valor_byte);

    } while(1);
}

int processa_controle( char controle ) {
    /* O comando putc_cdc envia o byte controle para o buffer de saida do
     * PIC. Nesse caso, esse byte sera diferente do enviado para o PIC, pois
     * os dois bits mais significativos foram transformados em 00 e, se fossem
     * iguais a 00 inicialmente, essa funcao nao teria sido chamada. */
    putc_cdc(controle);
    return 1;
}

int processa_dado( char dado ) {
    /* Ja' nesse caso, o byte enviado sera identico ao recebido. */
    putc_cdc(dado);
    return 1;
}
