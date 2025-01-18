/*
servidor_iterativo

 gcc -o servidor_iterativo servidor_iterativo.c -lws2_32
./servidor_iterativo 8080

*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <winsock2.h>
#include <windows.h>
#include <ctype.h>

#define WIN32_LEAN_AND_MEAN

// Exibir uma mensagem de erro e sair
void mensagemDeErro(const char *msg)
{
    perror(msg);
    exit(1);
}

// gerar a palavra oculta
void generateHiddenWord(const char *word, const int *revealed, char *hiddenWord)
{
    size_t length = strlen(word);
    for (size_t i = 0; i < length; ++i)
    {
        hiddenWord[i] = revealed[i] ? word[i] : '_';
    }
    hiddenWord[length] = '\0';
}

// verificar vitória
int checkVitoria(const int *revealed, size_t length)
{
    for (size_t i = 0; i < length; ++i)
    {
        if (!revealed[i])
        {
            return 0;
        }
    }
    return 1;
}

int main(int argc, char *argv[])
{
    WSADATA wsa;
    SOCKET sockfd, newsockfd;
    struct sockaddr_in serv_addr, cli_addr;
    int portno;
    int clilen;
    char buffer[1024];
    int n;

    SetConsoleOutputCP(CP_UTF8); // só para melhorar a visualização no console

    // verifica se o número da porta foi fornecido
    if (argc < 2)
    {
        fprintf(stderr, "ERROR, no port provided\n");
        exit(1);
    }

    // inicia o Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
    {
        printf("Erro na inicializacao do Winsock: %d\n", WSAGetLastError());
        exit(1);
    }

    // cia o socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == INVALID_SOCKET)
    {
        mensagemDeErro("Erro ao iniciar o socket");
    }

    memset((char *)&serv_addr, 0, sizeof(serv_addr));
    portno = atoi(argv[1]);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    // Faz o bind do socket ao endereço configurado
    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == SOCKET_ERROR)
    {
        mensagemDeErro("Erro no binding");
    }

    // Configura o socket para ouvir conexões
    listen(sockfd, 5);
    clilen = sizeof(cli_addr);

    // ====== JOGO DA FORCA =========

    const char word[] = "redes";
    size_t wordLength = strlen(word);
    int revealed[64] = {0};
    int attemptsLeft = 6;
    char hiddenWord[64];

    while (1)
    {
        printf("Esperando nova conexao\n");

        // aceita  nova conexao
        newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
        if (newsockfd == INVALID_SOCKET)
        {
            mensagemDeErro("Erro no accept");
        }

        printf("Conexao estabelecida\n");

        while (attemptsLeft > 0 && !checkVitoria(revealed, wordLength))
        {
            memset(buffer, 0, sizeof(buffer));
            n = recv(newsockfd, buffer, sizeof(buffer) - 1, 0);
            if (n <= 0)
            {
                printf("Conexao encerrada\n");
                break;
            }

            char guess = tolower(buffer[0]);
            int found = 0;

            for (size_t i = 0; i < wordLength; ++i)
            {
                if (word[i] == guess)
                {
                    revealed[i] = 1;
                    found = 1;
                }
            }

            if (!found)
            {
                --attemptsLeft;
            }

            // Gera a palavra oculta com as letras reveladas
            generateHiddenWord(word, revealed, hiddenWord);

            snprintf(buffer, sizeof(buffer), "\nPalavra: %s Tentativas restantes: %d", hiddenWord, attemptsLeft);

            if (checkVitoria(revealed, wordLength))
            {
                strcat(buffer, " GANHOU! ");
            }
            else if (attemptsLeft == 0)
            {
                snprintf(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer), " PERDEU! Resposta: %s", word);
            }
            else
            {
                strcat(buffer, " Digite uma letra: ");
            }

            n = send(newsockfd, buffer, strlen(buffer), 0);
            if (n < 0)
            {
                mensagemDeErro("Erro ao enviar resposta para o cliente");
            }
        }

        // Fecha a conexão
        closesocket(newsockfd);
        printf("Conexao encerrada\n");

        // Reinicia
        memset(revealed, 0, sizeof(revealed));
        attemptsLeft = 6;
    }

    // Fecha o socket principal e limpa o Winsock
    closesocket(sockfd);
    WSACleanup();
    return 0;
}
