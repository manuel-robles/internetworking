#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <process.h>

#define PORT 12345
#define MAX_CLIENTS 10
#define MAX_QUESTION_LENGTH 256
#define MAX_ANSWER_LENGTH 128

#pragma comment(lib, "ws2_32.lib")

typedef struct {
    char question[MAX_QUESTION_LENGTH];
    char answers[4][MAX_ANSWER_LENGTH];
    int correct_answer;
} Question;

typedef struct {
    SOCKET socket;
    int role;  // 0: Subir pregunta, 1: Responder pregunta
} ClientInfo;

Question questions[10];  // Array para almacenar las preguntas
int current_question = 0;  // Índice de la pregunta actual

void receive_question(ClientInfo *client) {
    Question received_question;
    recv(client->socket, (char*)&received_question, sizeof(Question), 0);

    // Almacena la pregunta recibida en el array
    questions[current_question++] = received_question;

    // Aquí puedes procesar la pregunta recibida, por ejemplo, mostrarla en el servidor
    printf("Pregunta recibida:\n");
    printf("Pregunta: %s\n", received_question.question);
    for (int i = 0; i < 4; ++i) {
        printf("Respuesta %d: %s\n", i + 1, received_question.answers[i]);
    }
    printf("Respuesta correcta: %d\n\n", received_question.correct_answer);
}

int calculate_score(char *client_answers, char *correct_answers) {
    int score = 0;
    for (int i = 0; i < strlen(correct_answers); ++i) {
        if (client_answers[i] == correct_answers[i]) {
            score++;
        }
    }
    return score;
}

void handle_client(void *arg) {
    ClientInfo *client = (ClientInfo*)arg;

    if (client->role == 0) {
        // Cliente que sube preguntas
        receive_question(client);
    } else {
        // Cliente que responde preguntas
        for (int i = 0; i < 10; ++i) {
            // Envía la pregunta al cliente
            send(client->socket, (const char*)&questions[i], sizeof(Question), 0);

            // Recibe las respuestas del cliente
            char client_answers[5];
            recv(client->socket, client_answers, sizeof(client_answers), 0);

            // Calcula el puntaje
            int score = calculate_score(client_answers, questions[i].answers[questions[i].correct_answer - 1]);

            // Envía el puntaje al cliente
            send(client->socket, (const char*)&score, sizeof(int), 0);
        }
    }

    printf("Cliente desconectado\n");
    closesocket(client->socket);
    free(client);
    _endthreadex(0);
}

int main() {
    WSADATA wsa;
    SOCKET server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    int addr_len;

    // Inicializar Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("Error al inicializar Winsock\n");
        return 1;
    }

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == INVALID_SOCKET) {
        printf("Error al crear el socket del servidor\n");
        WSACleanup();
        return 1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        printf("Error al enlazar el socket del servidor\n");
        closesocket(server_socket);
        WSACleanup();
        return 1;
    }

    if (listen(server_socket, MAX_CLIENTS) == SOCKET_ERROR) {
        printf("Error al escuchar en el socket del servidor\n");
        closesocket(server_socket);
        WSACleanup();
        return 1;
    }

    printf("Servidor esperando conexiones...\n");

    addr_len = sizeof(struct sockaddr_in);

    // Usar un bucle infinito para aceptar múltiples clientes
    while (1) {
        client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &addr_len);
        printf("Cliente conectado\n");

        // Crea una estructura para almacenar la información del cliente
        ClientInfo *client = (ClientInfo*)malloc(sizeof(ClientInfo));
        client->socket = client_socket;

        // Asigna un rol al cliente (0 o 1) alternadamente
        client->role = current_question % 2;

        // Crea un hilo para manejar el cliente concurrentemente
        _beginthread(handle_client, 0, (void*)client);

        current_question++;
    }

    closesocket(server_socket);
    WSACleanup();

    return 0;
}
