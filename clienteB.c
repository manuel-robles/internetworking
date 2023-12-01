#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>

#define SERVER_IP "127.0.0.1"
#define PORT 12345
#define MAX_QUESTION_LENGTH 256
#define MAX_ANSWER_LENGTH 128

#pragma comment(lib, "ws2_32.lib")

typedef struct {
    char question[MAX_QUESTION_LENGTH];
    char answers[4][MAX_ANSWER_LENGTH];
    int correct_answer;
} Question;

void send_question(SOCKET client_socket) {
    Question new_question;

    printf("Ingrese la pregunta: ");
    fgets(new_question.question, MAX_QUESTION_LENGTH, stdin);
    new_question.question[strlen(new_question.question) - 1] = '\0';  // Eliminar el salto de línea

    for (int i = 0; i < 4; ++i) {
        printf("Ingrese la respuesta %d: ", i + 1);
        fgets(new_question.answers[i], MAX_ANSWER_LENGTH, stdin);
        new_question.answers[i][strlen(new_question.answers[i]) - 1] = '\0';  // Eliminar el salto de línea
    }

    printf("Ingrese la respuesta correcta (1-4): ");
    scanf("%d", &new_question.correct_answer);
    getchar();  // Consumir el salto de línea residual

    send(client_socket, (const char*)&new_question, sizeof(Question), 0);
}

void receive_question_and_answer(SOCKET client_socket) {
    Question received_question;
    int score;

    for (int i = 0; i < 10; ++i) {
        // Recibe la pregunta del servidor
        recv(client_socket, (char*)&received_question, sizeof(Question), 0);

        // Muestra la pregunta al usuario y espera la respuesta
        printf("Pregunta recibida:\n");
        printf("Pregunta: %s\n", received_question.question);
        for (int j = 0; j < 4; ++j) {
            printf("Respuesta %d: %s\n", j + 1, received_question.answers[j]);
        }

        printf("Ingrese sus respuestas (por ejemplo, 1234): ");
        char client_answers[5];
        fgets(client_answers, sizeof(client_answers), stdin);

        // Envía las respuestas al servidor
        send(client_socket, client_answers, sizeof(client_answers), 0);

        // Recibe el puntaje del servidor
        recv(client_socket, (char*)&score, sizeof(int), 0);

        printf("Puntaje obtenido: %d\n\n", score);
    }
}

int main() {
    WSADATA wsa;
    SOCKET client_socket;
    struct sockaddr_in server_addr;

    // Inicializar Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("Error al inicializar Winsock\n");
        return 1;
    }

    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == INVALID_SOCKET) {
        printf("Error al crear el socket del cliente\n");
        WSACleanup();
        return 1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
    server_addr.sin_port = htons(PORT);

    if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        printf("Error al conectar con el servidor\n");
        closesocket(client_socket);
        WSACleanup();
        return 1;
    }

    // Cliente elige el rol
    int role;
    printf("Seleccione su rol (0 para subir preguntas, 1 para responder preguntas): ");
    scanf("%d", &role);

    if (role == 0) {
        // Subir preguntas
        send_question(client_socket);
    } else {
        // Responder preguntas
        receive_question_and_answer(client_socket);
    }

    closesocket(client_socket);
    WSACleanup();

    return 0;
}
