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

void send_question(SOCKET client_socket, const Question *question) {
    send(client_socket, (const char*)question, sizeof(Question), 0);
}

void receive_question(SOCKET client_socket, Question *received_question) {
    if (recv(client_socket, (char*)received_question, sizeof(Question), 0) <= 0) {
        printf("Error al recibir la pregunta o conexión cerrada.\n");
        exit(1);
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

    // Crear socket del cliente
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == INVALID_SOCKET) {
        printf("Error al crear el socket del cliente\n");
        WSACleanup();
        return 1;
    }

    // Configurar la dirección del servidor
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
    server_addr.sin_port = htons(PORT);

    // Conectar al servidor
    if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        printf("Error al conectar con el servidor\n");
        closesocket(client_socket);
        WSACleanup();
        return 1;
    }

    do {
        // Rol del cliente (0 o 1)
        int role;
        printf("\nSeleccione su rol (0 para subir preguntas, 1 para responder preguntas, -1 para salir): ");
        scanf("%d", &role);

        if (role == -1) {
            // Salir del programa
            break;
        }

        send(client_socket, (const char*)&role, sizeof(int), 0);

        if (role == 0) {
            // Cliente con rol 0 (Subir preguntas)
            Question questions[10];

            int num_questions;
            do {
                // Limpiar el búfer del teclado antes de cada lectura
                fflush(stdin);

                printf("¿Cuántas preguntas desea subir (máximo 10)? ");
                scanf("%d", &num_questions);

                if (num_questions > 10 || num_questions <= 0) {
                    printf("Por favor, ingrese un número válido (entre 1 y 10).\n");
                }
            } while (num_questions > 10 || num_questions <= 0);

            for (int i = 0; i < num_questions; ++i) {
                // Limpiar el búfer del teclado antes de cada lectura
                fflush(stdin);

                printf("Ingrese la pregunta %d: ", i + 1);
                scanf(" %[^\n]", questions[i].question);

                for (int j = 0; j < 4; ++j) {
                    printf("Ingrese la respuesta %d para la pregunta %d: ", j + 1, i + 1);
                    scanf(" %[^\n]", questions[i].answers[j]);
                }

                printf("Ingrese la respuesta correcta (1-4) para la pregunta %d: ", i + 1);
                scanf("%d", &questions[i].correct_answer);

                // Enviar la pregunta al servidor
                send_question(client_socket, &questions[i]);

                // Esperar confirmación del servidor antes de enviar la siguiente pregunta
                char confirmation;
                recv(client_socket, &confirmation, sizeof(char), 0);
            }
        } else if (role == 1) {
            // Cliente con rol 1 (Recibir y mostrar preguntas)
            Question received_questions[10];  // Array para almacenar hasta 10 preguntas

            for (int i = 0; i < 10; ++i) {
                // Recibir pregunta del servidor
                receive_question(client_socket, &received_questions[i]);
                // Enviar confirmación al servidor para indicar que se recibió la pregunta
                char confirmation = '1';
                send(client_socket, &confirmation, sizeof(char), 0);
            }

            // Mostrar preguntas al cliente con rol 1
            printf("\nPreguntas recibidas desde el servidor:\n");
            for (int i = 0; i < 10; ++i) {
                if (strlen(received_questions[i].question) > 0) {
                    printf("Pregunta %d: %s\n", i + 1, received_questions[i].question);
                }
            }
        }
    } while (1);

    // Cerrar socket y limpiar Winsock
    closesocket(client_socket);
    WSACleanup();

    return 0;
}
