#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib")

#define PORT 8080
#define BUFFER_SIZE 1024

// inicializa o Winsock
int initialize_winsock(WSADATA *wsaData)
{
    if (WSAStartup(MAKEWORD(2, 2), wsaData) != 0)
    {
        printf("Erro ao iniciar Winsock.\n");
        return 0;
    }
    return 1;
}

// cria e configura o socket
SOCKET create_socket()
{
    SOCKET connection_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (connection_socket == INVALID_SOCKET)
    {
        printf("Erro para criar socket.\n");
        return INVALID_SOCKET;
    }
    return connection_socket;
}

// configura o servidor
int configure_server_address(struct sockaddr_in *server_address)
{
    server_address->sin_family = AF_INET;
    server_address->sin_addr.s_addr = INADDR_ANY;
    server_address->sin_port = htons(PORT);
    return 1;
}

// bind do socket
int bind_socket(SOCKET connection_socket, struct sockaddr_in *server_address)
{
    if (bind(connection_socket, (struct sockaddr *)server_address, sizeof(*server_address)) == SOCKET_ERROR)
    {
        printf("Erro no bind.\n");
        return 0;
    }
    return 1;
}

// escutar conexões
int start_listening(SOCKET connection_socket)
{
    if (listen(connection_socket, 5) == SOCKET_ERROR)
    {
        printf("Erro no listen.\n");
        return 0;
    }
    return 1;
}

// enviar uma resposta ao cliente
void send_response(SOCKET client_socket)
{
    char *response =
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html\r\n"
        "Content-Length: 19\r\n"
        "\r\n"
        "<h1>Hello World</h1>";
    send(client_socket, response, strlen(response), 0);
}

int main()
{
    WSADATA wsaData;
    SOCKET connection_socket, client_socket;
    struct sockaddr_in server_address, client_address;
    char buffer[BUFFER_SIZE];
    int address_length = sizeof(client_address);

    if (!initialize_winsock(&wsaData))
    {
        return 1;
    }

    connection_socket = create_socket();
    if (connection_socket == INVALID_SOCKET)
    {
        WSACleanup();
        return 1;
    }

    configure_server_address(&server_address);

    if (!bind_socket(connection_socket, &server_address))
    {
        closesocket(connection_socket);
        WSACleanup();
        return 1;
    }

    if (!start_listening(connection_socket))
    {
        closesocket(connection_socket);
        WSACleanup();
        return 1;
    }

    printf("Servidor iterativo na porta %d\n", PORT);

    while (1)
    {
        client_socket = accept(connection_socket, (struct sockaddr *)&client_address, &address_length);
        if (client_socket == INVALID_SOCKET)
        {
            printf("Erro ao aceitar conexao.\n");
            continue; // tenta aceitar outra conexão
        }

        // Recebe a solicitação, envia a resposta e fecha a conexão com o cliente
        recv(client_socket, buffer, BUFFER_SIZE, 0);
        send_response(client_socket);
        closesocket(client_socket);
    }

    // fecha o socket, limpa o winsock e termina o programa
    closesocket(connection_socket);
    WSACleanup();
    return 0;
}
