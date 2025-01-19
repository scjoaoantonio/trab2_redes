#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>

#define PORT 2020
#define BUF_SIZE 1024
#define MAX_CLIENTS 10

void handle_client_request(int client_fd) {
    char buf[BUF_SIZE];
    int nbytes = recv(client_fd, buf, sizeof(buf), 0);
    if (nbytes <= 0) {
        if (nbytes == 0)
            printf("Cliente desconectado no socket %d.\n", client_fd);
        else
            perror("recv() error");
        close(client_fd);
    } else {
        printf("Received request:\n%s\n", buf);

        // Resposta HTTP simples
        const char *response =
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/plain\r\n"
            "Content-Length: 13\r\n"
            "\r\n"
            "Hello, world!";
        send(client_fd, response, strlen(response), 0);
        close(client_fd);
    }
}

int main() {
    int listener, newfd, fdmax;
    struct sockaddr_in serveraddr, clientaddr;
    socklen_t addrlen;
    char buf[BUF_SIZE];
    fd_set master, read_fds;

    // Criação do socket de escuta
    if ((listener = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket() error");
        exit(1);
    }

    // Permitir reutilização de endereço
    int yes = 1;
    if (setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
        perror("setsockopt() error");
        exit(1);
    }

    // Configuração do servidor
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = INADDR_ANY;
    serveraddr.sin_port = htons(PORT);
    memset(&(serveraddr.sin_zero), '\0', 8);

    if (bind(listener, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) == -1) {
        perror("bind() error");
        exit(1);
    }

    // Colocando o servidor em modo de escuta
    if (listen(listener, MAX_CLIENTS) == -1) {
        perror("listen() error");
        exit(1);
    }

    // Inicializar a lista de descritores de arquivo
    FD_ZERO(&master);
    FD_ZERO(&read_fds);
    FD_SET(listener, &master); 
    fdmax = listener; 

    // Loop principal do servidor
    for (;;) {
        read_fds = master;

        // Usar select para esperar por atividade nos sockets
        if (select(fdmax + 1, &read_fds, NULL, NULL, NULL) == -1) {
            perror("select() error");
            exit(1);
        }

        // Verificar todos os sockets
        for (int i = 0; i <= fdmax; i++) {
            if (FD_ISSET(i, &read_fds)) {
                if (i == listener) {
                    // Novo cliente tentando se conectar
                    addrlen = sizeof(clientaddr);
                    if ((newfd = accept(listener, (struct sockaddr *)&clientaddr, &addrlen)) == -1) {
                        perror("accept() error");
                    } else {
                        FD_SET(newfd, &master); // Adicionar novo socket à lista
                        if (newfd > fdmax) {
                            fdmax = newfd;
                        }
                        printf("New connection from %s on socket %d\n",
                               inet_ntoa(clientaddr.sin_addr), newfd);
                    }
                } else {
                    // Atividade em um socket já conectado
                    handle_client_request(i);
                    FD_CLR(i, &master); // Remover o socket da lista de monitoramento
                }
            }
        }
    }

    return 0;
}
