
// void tentarSolucaoCompleta(char tentativaAtual[], char valoresCorretos[])
// {
//     for (int i = 0; i < strlen(tentativaAtual); i++)
//     {
//         if ((tentativaAtual[i] != '0') && (tentativaAtual[i] != valoresCorretos[i]))
//         {
//             char numero = tentativaAtual[i];
//             int numeroInt = (int)(numero);
//             int novoNumero = numeroInt + 1;
//             char novoNumeroChar = (char)(novoNumero);
//             tentativaAtual[i] = novoNumeroChar;
//         }
//         else if (tentativaAtual[i] == '0')
//         {
//             tentativaAtual[i] = '1';
//         }
//     }
// }

// // atualiza os valoresCorretos da Ultima Tentativa
// void atualizaValoresCorretosCompletos(char tentativaAtual[], char valoresCorretos[], char solucao[], int nTentativas)
// {
//     char Tentativas[100];
//     sprintf(Tentativas, "Tentativa n: %d \n", nTentativas);
//     logEventoCliente(Tentativas);
//     for (int i = 0; i < strlen(tentativaAtual); i++)
//     {
//         if (valoresCorretos[i] == '0')
//         {
//             if (tentativaAtual[i] == solucao[i])
//             {
//                 valoresCorretos[i] = tentativaAtual[i];
//                 char message[1024];
//                 sprintf(message, "Valor correto(%d), na posição %d da String \n", tentativaAtual[i], i + 1);
//                 logEventoCliente(message);
//                 printf(message);

//                 // printf("%d \n", valoresCorretos);
//             }
//             else
//             {
//                 char message[1024];
//                 sprintf(message, "Valor incorreto(%d), na posição %d da String \n", tentativaAtual[i], i + 1);
//                 logEventoCliente(message);
//                 printf(message);
//             }
//         }
//     }
// }

// // // Atualiza o booleano Resolvido se o jogo tiver sido resolvido
// bool verificaResolvido(char valoresCorretos[], char solucao[], bool resolvido)
// {
//     for (int i = 0; i < strlen(valoresCorretos); i++)
//     {
//         if (valoresCorretos[i] != solucao[i])
//         {
//             return false;
//         }
//     }
//     return true;
// }

// // ResolveJogo
// void resolverJogoCompleto(char jogo[], char solucao[], int nTentativas)
// {
//     char tentativaAtual[NUMEROS_NO_JOGO];
//     char valoresCorretos[NUMEROS_NO_JOGO];
//     strncpy(tentativaAtual, jogo, NUMEROS_NO_JOGO);
//     strncpy(valoresCorretos, jogo, NUMEROS_NO_JOGO);
//     bool resolvido;
//     resolvido = false;
//     printf("Jogo Inicial: \n \n");
//     imprimirTabuleiro(jogo);
//     while (!resolvido)
//     {
//         // for(int i = 0; i< 10; i++){

//         nTentativas = nTentativas + 1;
//         tentarSolucaoCompleta(tentativaAtual, valoresCorretos);
//         atualizaValoresCorretosCompletos(tentativaAtual, valoresCorretos, solucao, nTentativas);
//         resolvido = verificaResolvido(valoresCorretos, solucao, resolvido);

//         printf("tentativaAtual: \n");
//         imprimirTabuleiro(tentativaAtual);

//         printf("ValoresCorretos: \n");
//         imprimirTabuleiro(valoresCorretos);

//         printf("Solução obtida até o momento: \n");
//         imprimirTabuleiro(valoresCorretos);
//     }
//     //}
//     printf("Parabéns, esta é a resolução correta! \n");
//     char TentativasTotais[100];
//     sprintf(TentativasTotais, "Tentativas totais: %d \n", nTentativas);
//     logEventoCliente(TentativasTotais);
//     printf(TentativasTotais);
// }

// // Solucao parcial (Tentar um valor por vez)
// void tentarSolucaoParcial(char tentativaAtual[], char valoresCorretos[])
// {
//     for (int i = 0; i < strlen(tentativaAtual); i++)
//     {
//         if ((tentativaAtual[i] != '0') && (tentativaAtual[i] != valoresCorretos[i]))
//         {
//             char numero = tentativaAtual[i];
//             int numeroInt = (int)(numero);
//             int novoNumero = numeroInt + 1;
//             char novoNumeroChar = (char)(novoNumero);
//             tentativaAtual[i] = novoNumeroChar;
//             break;
//         }
//         else if (tentativaAtual[i] == '0')
//         {
//             tentativaAtual[i] = '1';
//             break;
//         }
//     }
// }

// void atualizaValoresCorretosParcial(char tentativaAtual[], char valoresCorretos[], char solucao[], int nTentativas)
// {
//     char Tentativas[100];
//     sprintf(Tentativas, "Tentativa n: %d \n", nTentativas);
//     logEventoCliente(Tentativas);
//     for (int i = 0; i < strlen(tentativaAtual); i++)
//     {
//         if (valoresCorretos[i] == '0')
//         {
//             if (tentativaAtual[i] == solucao[i])
//             {
//                 valoresCorretos[i] = tentativaAtual[i];
//                 char message[1024];
//                 sprintf(message, "Valor correto(%d), na posição %d da String \n", tentativaAtual[i], i + 1);
//                 logEventoCliente(message);
//                 printf(message);
//                 return;
//                 // printf("%d \n", valoresCorretos);
//             }
//             else
//             {
//                 char message[1024];
//                 sprintf(message, "Valor incorreto(%d), na posição %d da String \n", tentativaAtual[i], i + 1);
//                 logEventoCliente(message);
//                 printf(message);
//                 return;
//             }
//         }
//     }
// }
// //ResolveJogo
// void resolverJogoParcial(char jogo[], char solucao[], int nTentativas)
// {
//     char tentativaAtual[NUMEROS_NO_JOGO];
//     char valoresCorretos[NUMEROS_NO_JOGO];
//     strncpy(tentativaAtual, jogo, NUMEROS_NO_JOGO);
//     strncpy(valoresCorretos, jogo, NUMEROS_NO_JOGO);
//     bool resolvido;
//     resolvido = false;
//     printf("Jogo Inicial: \n \n");
//     imprimirTabuleiro(jogo);
//     while (!resolvido)
//     {
//         // for(int i = 0; i< 10; i++){
//         nTentativas = nTentativas + 1;
//         tentarSolucaoParcial(tentativaAtual, valoresCorretos);
//         atualizaValoresCorretosParcial(tentativaAtual, valoresCorretos, solucao, nTentativas);
//         resolvido = verificaResolvido(valoresCorretos, solucao, resolvido);

//         printf("tentativaAtual: \n");
//         imprimirTabuleiro(tentativaAtual);

//         printf("ValoresCorretos: \n");
//         imprimirTabuleiro(valoresCorretos);

//         printf("Solução obtida até o momento: \n");
//         imprimirTabuleiro(valoresCorretos);
//     }
//     //}
//     printf("Parabéns, esta é a resolução correta! \n");
//     char TentativasTotais[100];
//     sprintf(TentativasTotais, "Tentativas totais: %d \n", nTentativas);
//     logEventoCliente(TentativasTotais);
//     printf(TentativasTotais);
// }