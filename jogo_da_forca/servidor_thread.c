// gcc -o servidor_thread servidor_thread.c -lws2_32 -lpthread
// ./servidor_thread 8080

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <windows.h>
#include <pthread.h>
#include <ctype.h>

volatile int running = 1;

void mensagemDeErro(const char *msg)
{
    fprintf(stderr, "%s: %d\n", msg, WSAGetLastError());
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

void *handle_client(void *arg)
{
    int newsockfd = *(int *)arg;
    free(arg);

    const char word[] = "redes";
    size_t wordLength = strlen(word);
    int revealed[64] = {0};
    int attemptsLeft = 6;
    char hiddenWord[64];

    generateHiddenWord(word, revealed, hiddenWord);

    while (attemptsLeft > 0 && !checkVitoria(revealed, wordLength))
    {
        char buffer[1024] = {0};
        snprintf(buffer, sizeof(buffer), "Palavra: %s\nTentativas restantes: %d\nDigite uma letra:\n", hiddenWord, attemptsLeft);
        send(newsockfd, buffer, strlen(buffer), 0);

        memset(buffer, 0, sizeof(buffer));
        int n = recv(newsockfd, buffer, sizeof(buffer) - 1, 0);
        if (n <= 0)
        {
            printf("Cliente desconectado.\n");
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
            send(newsockfd, "Errado!\n", 8, 0);
        }
        else
        {
            send(newsockfd, "Certo!\n", 7, 0);
        }

        generateHiddenWord(word, revealed, hiddenWord);
    }

    if (checkVitoria(revealed, wordLength))
    {
        send(newsockfd, "GANHOU!\n", 8, 0);
    }
    else
    {
        char buffer[1024];
        snprintf(buffer, sizeof(buffer), "PERDEU! Resposta: %s\n", word);
        send(newsockfd, buffer, strlen(buffer), 0);
    }

    closesocket(newsockfd);
    return NULL;
}

int main(int argc, char *argv[])
{
    WSADATA wsaData;
    int sockfd, newsockfd, portno;
    struct sockaddr_in serv_addr, cli_addr;
    int clilen;

    if (argc < 2)
    {
        fprintf(stderr, "ERRO, nenhuma porta foi fornecida\n");
        exit(1);
    }

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
        mensagemDeErro("Falha ao inicializar Winsock");

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == INVALID_SOCKET)
        mensagemDeErro("Erro ao abrir o socket");

    memset((char *)&serv_addr, 0, sizeof(serv_addr));
    portno = atoi(argv[1]);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == SOCKET_ERROR)
        mensagemDeErro("Erro no binding");

    listen(sockfd, 5);
    clilen = sizeof(cli_addr);

    printf("Servidor iniciado na porta %d\n", portno);

    while (running)
    {
        printf("Aguardando conexao...\n");
        newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
        if (newsockfd == INVALID_SOCKET)
        {
            fprintf(stderr, "Erro no accept: %d\n", WSAGetLastError());
            continue;
        }

        printf("Conexao estabelecida com um cliente!\n");

        int *arg = malloc(sizeof(int));
        *arg = newsockfd;
        pthread_t thread;
        if (pthread_create(&thread, NULL, handle_client, arg) != 0)
        {
            fprintf(stderr, "Erro ao criar a thread\n");
            free(arg);
            closesocket(newsockfd);
        }

        pthread_detach(thread);
    }

    closesocket(sockfd);
    WSACleanup();
    printf("Servidor encerrado.\n");
    return 0;
}
