// para compilar: gcc -o server server.c task_queue.c -pthread
// para executar: ./server

#include "task_queue.h" 

#define NUM_THREADS 4
#define PORT 8080

// Função para processar a requisição HTTP
void process_request(int client_socket) {
    char buffer[1024];
    int bytes_read = read(client_socket, buffer, sizeof(buffer));

    if (bytes_read > 0) {
        const char *response = "HTTP/1.1 200 OK\nContent-Type: text/html\n\n<html><body><h1>Olá, Mundo!</h1></body></html>";
        write(client_socket, response, strlen(response));
    }
    close(client_socket);
}

// Função que será executada pelas threads para processar as requisições
void *worker_thread(void *arg) {
    while (1) {
        task_t task = dequeue(); 
        process_request(task.socket_fd);  
    }
    return NULL;
}

int setup_server() {
    int server_socket;
    struct sockaddr_in server_addr;

    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Erro ao criar socket");
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Erro ao associar o socket");
        exit(EXIT_FAILURE);
    }

    if (listen(server_socket, 5) < 0) {
        perror("Erro ao escutar no socket");
        exit(EXIT_FAILURE);
    }

    return server_socket;
}

// Função para aceitar conexões e enfileirar as tarefas
void accept_connections(int server_socket) {
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    int client_socket;

    while (1) {
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_len);
        if (client_socket < 0) {
            perror("Erro ao aceitar a conexão");
            continue;
        }

        task_t task = {client_socket};
        enqueue(task); 
    }
}

int main() {
    int server_socket = setup_server();

    // Criar as threads de processamento
    pthread_t threads[NUM_THREADS];
    for (int i = 0; i < NUM_THREADS; i++) {
        if (pthread_create(&threads[i], NULL, worker_thread, NULL) != 0) {
            perror("Erro ao criar thread");
            exit(EXIT_FAILURE);
        }
    }

    accept_connections(server_socket);

    close(server_socket);
    return 0;
}
