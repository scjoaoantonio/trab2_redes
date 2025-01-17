#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

void generateHiddenWord(const char *word, const int *revealed, char *hiddenWord)
{
    size_t length = strlen(word);
    for (size_t i = 0; i < length; ++i)
    {
        hiddenWord[i] = revealed[i] ? word[i] : '_';
    }
    hiddenWord[length] = '\0';
}

int checkWin(const int *revealed, size_t length)
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

    SetConsoleOutputCP(CP_UTF8);

    if (argc < 2)
    {
        fprintf(stderr, "ERROR, no port provided\n");
        exit(1);
    }

    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
    {
        printf("Erro ao inicializar o Winsock: %d\n", WSAGetLastError());
        exit(1);
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == INVALID_SOCKET)
    {
        error("Erro ao abrir o socket");
    }

    memset((char *)&serv_addr, 0, sizeof(serv_addr));
    portno = atoi(argv[1]);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == SOCKET_ERROR)
    {
        error("Erro no binding");
    }

    listen(sockfd, 5);
    clilen = sizeof(cli_addr);

    const char word[] = "forca"; // Palavra fixa para o jogo
    size_t wordLength = strlen(word);
    int revealed[64] = {0}; // Tamanho fixo suficiente para a palavra
    int attemptsLeft = 6;
    char hiddenWord[64];

    while (1)
    {
        printf("Servidor esperando uma nova conexão...\n");

        newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
        if (newsockfd == INVALID_SOCKET)
        {
            error("Erro no accept");
        }

        printf("Conexão estabelecida com um cliente!\n");

        while (attemptsLeft > 0 && !checkWin(revealed, wordLength))
        {
            memset(buffer, 0, sizeof(buffer));
            n = recv(newsockfd, buffer, sizeof(buffer) - 1, 0);
            if (n <= 0)
            {
                printf("Conexão encerrada pelo cliente.\n");
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
            generateHiddenWord(word, revealed, hiddenWord);
            snprintf(buffer, sizeof(buffer), "\nPalavra: %s Tentativas restantes: %d", hiddenWord, attemptsLeft);

            if (checkWin(revealed, wordLength))
            {
                strcat(buffer, " Parabéns! Você ganhou!");
            }
            else if (attemptsLeft == 0)
            {
                snprintf(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer), " Você perdeu! Resposta: %s", word);
            }
            else
            {
                strcat(buffer, " Digite uma letra: ");
            }

            // Envia o buffer ao cliente
            n = send(newsockfd, buffer, strlen(buffer), 0);
            if (n < 0)
            {
                error("Erro ao enviar resposta para o cliente");
            }
        }

        closesocket(newsockfd);
        printf("Conexão encerrada. Reiniciando o jogo.\n");
        memset(revealed, 0, sizeof(revealed));
        attemptsLeft = 6;
    }

    closesocket(sockfd);
    WSACleanup();
    return 0;
}
