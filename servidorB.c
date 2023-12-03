#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>

#define PORT 12345
#define MAX_QUESTION_LENGTH 256
#define MAX_ANSWER_LENGTH 128

#pragma comment(lib, "ws2_32.lib")

typedef struct {
    char question[MAX_QUESTION_LENGTH];
    char answers[4][MAX_ANSWER_LENGTH];
    int correct_answer;
} Question;

void receive_question(SOCKET client_socket, Question *received_question) {
    if (recv(client_socket, (char*)received_question, sizeof(Question), 0) <= 0) {
        printf("Error al recibir la pregunta o conexión cerrada.\n");
        exit(1);
    }
}

void send_question(SOCKET client_socket, const Question *question) {
    send(client_socket, (const char*)question, sizeof(Question), 0);
}

int main() {
    WSADATA wsa;
    SOCKET server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    int addr_len = sizeof(client_addr);

    // Inicializar Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("Error al inicializar Winsock\n");
        return 1;
    }

    // Crear socket del servidor
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == INVALID_SOCKET) {
        printf("Error al crear el socket del servidor\n");
        WSACleanup();
        return 1;
    }

    // Configurar la dirección del servidor
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Vincular el socket a la dirección del servidor
    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        printf("Error al vincular el socket\n");
        closesocket(server_socket);
        WSACleanup();
        return 1;
    }

    // Escuchar conexiones entrantes
    if (listen(server_socket, 1) == SOCKET_ERROR) {
        printf("Error al configurar el modo de escucha\n");
        closesocket(server_socket);
        WSACleanup();
        return 1;
    }

    printf("Esperando conexiones...\n");

    // Aceptar conexión entrante
    client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &addr_len);
    if (client_socket == INVALID_SOCKET) {
        printf("Error al aceptar la conexión\n");
        closesocket(server_socket);
        WSACleanup();
        return 1;
    }

    // Rol del cliente (0 o 1)
    int role;
    recv(client_socket, (char*)&role, sizeof(int), 0);

    if (role == 0) {
        // Cliente con rol 0 (Subir preguntas)
        Question questions[10];  // Array para almacenar hasta 10 preguntas

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
            // Recibir pregunta del cliente con rol 0
            receive_question(client_socket, &received_questions[i]);
            // Enviar confirmación al cliente con rol 0 para indicar que se recibió la pregunta
            char confirmation = '1';
            send(client_socket, &confirmation, sizeof(char), 0);
        }

        // Mostrar preguntas al cliente con rol 1
        printf("\nPreguntas recibidas desde el cliente con rol 0:\n");
        for (int i = 0; i < 10; ++i) {
            if (strlen(received_questions[i].question) > 0) {
                printf("Pregunta %d: %s\n", i + 1, received_questions[i].question);
            }
        }
    }

    // Cerrar sockets y limpiar Winsock
    closesocket(client_socket);
    closesocket(server_socket);
    WSACleanup();

    return 0;
}
