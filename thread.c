#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <windows.h>

#pragma comment(lib, "ws2_32.lib")

#define PORT 8080
#define BUFFER_SIZE 1024

// cria uma thread separada para o cliente
DWORD WINAPI handle_client(LPVOID client_socket)
{
    SOCKET client_fd = *(SOCKET *)client_socket;
    char buffer[BUFFER_SIZE];
    char *response =
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html\r\n"
        "Content-Length: 19\r\n"
        "\r\n"
        "<h1>Hello World</h1>";

    // Recebe a solicitação, envia a resposta e fecha a conexão com o cliente
    recv(client_fd, buffer, BUFFER_SIZE, 0);
    send(client_fd, response, strlen(response), 0);
    closesocket(client_fd);
    free(client_socket);
    return 0;
}

// inicializar o Winsock
int initialize_winsock(WSADATA *wsaData)
{
    if (WSAStartup(MAKEWORD(2, 2), wsaData) != 0)
    {
        fprintf(stderr, "Erro ao iniciar Winsock.\n");
        return 0;
    }
    return 1;
}

// criar e configurar o socket
SOCKET create_socket()
{
    SOCKET server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == INVALID_SOCKET)
    {
        fprintf(stderr, "Erro ao criar socket.\n");
        return INVALID_SOCKET;
    }
    return server_fd;
}

// configurar o do servidor
void configure_server_address(struct sockaddr_in *server_addr)
{
    server_addr->sin_family = AF_INET;
    server_addr->sin_addr.s_addr = INADDR_ANY;
    server_addr->sin_port = htons(PORT);
}

// bind do socket
int bind_socket(SOCKET server_fd, struct sockaddr_in *server_addr)
{
    if (bind(server_fd, (struct sockaddr *)server_addr, sizeof(*server_addr)) == SOCKET_ERROR)
    {
        fprintf(stderr, "Erro no bind.\n");
        return 0;
    }
    return 1;
}

// escutar e aceitar conexões
int start_listening(SOCKET server_fd)
{
    if (listen(server_fd, 5) == SOCKET_ERROR)
    {
        fprintf(stderr, "Erro no listen.\n");
        return 0;
    }
    return 1;
}

int main()
{
    WSADATA wsaData;
    SOCKET server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    int addr_len = sizeof(client_addr);

    if (!initialize_winsock(&wsaData))
    {
        return 1;
    }

    server_fd = create_socket();
    if (server_fd == INVALID_SOCKET)
    {
        WSACleanup();
        return 1;
    }

    configure_server_address(&server_addr);

    if (!bind_socket(server_fd, &server_addr))
    {
        closesocket(server_fd);
        WSACleanup();
        return 1;
    }

    if (!start_listening(server_fd))
    {
        closesocket(server_fd);
        WSACleanup();
        return 1;
    }

    printf("Servidor escutando na porta %d...\n", PORT);

    while (1)
    {
        // Aceita uma conexão
        client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &addr_len);
        if (client_fd == INVALID_SOCKET)
        {
            fprintf(stderr, "Erro ao aceitar conexão.\n");
            continue; // Tenta aceitar outra conexão
        }

        // Aloca memória para o socket para passagem de argumento para a thread
        SOCKET *client_socket = malloc(sizeof(SOCKET));
        if (client_socket == NULL)
        {
            fprintf(stderr, "Erro ao alocar memória.\n");
            closesocket(client_fd);
            continue;
        }
        *client_socket = client_fd;

        HANDLE thread_handle = CreateThread(
            NULL,
            0,
            handle_client,
            client_socket,
            0,
            NULL);

        if (thread_handle == NULL)
        {
            fprintf(stderr, "Erro ao criar thread.\n");
            closesocket(client_fd);
            free(client_socket);
        }
        else
        {
            CloseHandle(thread_handle);
        }
    }

    closesocket(server_fd);
    WSACleanup();
    return 0;
}
