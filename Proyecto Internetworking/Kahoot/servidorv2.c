#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <windows.h>
#include <ctype.h>

#define PORT 8080
#define MAX_CLIENTS 10
#define MAX_QUESTION_LEN 256
#define MAX_ANSWER_LEN 256
#define TIMEOUT_SECONDS 10

void handle_client(SOCKET client_socket) {
    char question[MAX_QUESTION_LEN];
    char answer[MAX_ANSWER_LEN];

    // Preguntas y respuestas
    char questions_answers[][2][256] = {
        {"El tamaño del header IP es fijo medido en palabras, ¿de cuantos bits?", "32"},
        {"¿IW se aplica a LAN, WAN o ambos?", "ambos"},
        {"¿en que capa del modelo OSI se definen las direcciones IP?", "3"}
        
    };

    int score = 0;

    // Bienvenida
    send(client_socket, "¡Bienvenido a la trivia Kahoot!\n", sizeof("¡Bienvenido a la trivia Kahoot!\n"), 0);

    // Preguntas y respuestas
    for (int i = 0; i < sizeof(questions_answers) / sizeof(questions_answers[0]); ++i) {
        strcpy(question, questions_answers[i][0]);
        strcpy(answer, questions_answers[i][1]);

        // Enviar la pregunta al cliente
        send(client_socket, question, sizeof(question), 0);

        // Imprimir mensaje de temporizador
        printf("El cliente tiene %d segundos para responder.\n", TIMEOUT_SECONDS);

        // Configurar temporizador
        Sleep(TIMEOUT_SECONDS * 1000);

        // Recibir la respuesta del cliente
        char client_response[MAX_ANSWER_LEN];
        int bytes_received = recv(client_socket, client_response, sizeof(client_response), 0);
        client_response[bytes_received] = '\0';

        //Convertimos la respuesta para que no importe si es con may o min
        //probar
       for (int j=0; j<strlen(client_response); ++j){
            client_response[j]=tolower(client_response[j]);
        }

        // Verificar la respuesta
        if (strcmp(client_response, answer) == 0) {
            // Respuesta correcta
            send(client_socket, "Correcto!\n", sizeof("Correcto!\n"), 0);
            score++;
        } else {
            // Respuesta incorrecta
            send(client_socket, "Incorrecto.\n", sizeof("Incorrecto.\n"), 0);
        }
    }

    // Enviar puntaje final al cliente
    char final_score[50];
    sprintf(final_score, "Tu puntaje final es: %d\n", score);
    send(client_socket, final_score, sizeof(final_score), 0);

    // Cerrar el socket del cliente
    closesocket(client_socket);
}

int main() {
    WSADATA wsa;
    SOCKET server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    int client_addrlen = sizeof(client_addr);

    // Inicializar la biblioteca de sockets de Windows
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        perror("Error al inicializar Winsock");
        exit(EXIT_FAILURE);
    }

    // Crear un socket del servidor
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == INVALID_SOCKET) {
        perror("Error al crear el socket del servidor");
        exit(EXIT_FAILURE);
    }

    // Configurar la dirección del servidor
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Enlazar el socket del servidor a la dirección y puerto
    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        perror("Error al enlazar el socket del servidor");
        closesocket(server_socket);
        WSACleanup();
        exit(EXIT_FAILURE);
    }

    // Escuchar conexiones entrantes
    if (listen(server_socket, MAX_CLIENTS) == SOCKET_ERROR) {
        perror("Error al escuchar conexiones entrantes");
        closesocket(server_socket);
        WSACleanup();
        exit(EXIT_FAILURE);
    }

    printf("Servidor esperando conexión en el puerto %d...\n", PORT);

    // Aceptar conexiones entrantes y manejar a los clientes
    while (1) {
        client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_addrlen);
        if (client_socket == INVALID_SOCKET) {
            perror("Error al aceptar la conexión del cliente");
            closesocket(server_socket);
            WSACleanup();
            exit(EXIT_FAILURE);
        }

        printf("Cliente conectado desde %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        // Manejar al cliente en un hilo o proceso separado
        handle_client(client_socket);
    }

    // Cerrar el socket del servidor
    closesocket(server_socket);

    // Cerrar la biblioteca de sockets de Windows
    WSACleanup();

    return 0;
}
