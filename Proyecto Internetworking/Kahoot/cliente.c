#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>

#define PORT 8080
#define BUFFER_SIZE 256

int main() {
    WSADATA wsa;
    SOCKET client_socket;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];

    // Inicializar la biblioteca de sockets de Windows
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        perror("Error al inicializar Winsock");
        exit(EXIT_FAILURE);
    }

    // Crear un socket del cliente
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == INVALID_SOCKET) {
        perror("Error al crear el socket del cliente");
        exit(EXIT_FAILURE);
    }

    // Configurar la dirección del servidor
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("192.168.100.68");  // Reemplaza con la dirección IP del servidor
    server_addr.sin_port = htons(PORT);

    // Conectar al servidor
    if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        perror("Error al conectar al servidor");
        closesocket(client_socket);
        WSACleanup();
        exit(EXIT_FAILURE);
    }

    printf("Conectado al servidor en %s:%d\n", inet_ntoa(server_addr.sin_addr), ntohs(server_addr.sin_port));

    // Recibir bienvenida del servidor
    int bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
    buffer[bytes_received] = '\0';
    printf("%s", buffer);

    // Preguntas y respuestas
    while (1) {
        bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
        buffer[bytes_received] = '\0';

        // Verificar si se recibió el puntaje final
        if (strstr(buffer, "Tu puntaje final") != NULL) {
            printf("%s", buffer);
            break;
        }

        // Mostrar la pregunta y esperar la respuesta del usuario
        printf("%s", buffer);
        printf("Tienes 10 segundos para responder.\n");

        // Configurar temporizador de 10 segundos
        int timer = 10;
        while (timer > 0) {
            printf("Tiempo restante: %d segundos\n", timer);
            Sleep(1000);  // Esperar 1 segundo
            timer--;
        }

        // Limpiar el búfer de entrada antes de recibir la respuesta del usuario
        fflush(stdin);

        printf("Tu respuesta: ");
        fgets(buffer, sizeof(buffer), stdin);

        // Enviar la respuesta al servidor
        send(client_socket, buffer, strlen(buffer), 0);
    }

    // Cerrar el socket del cliente
    closesocket(client_socket);

    // Cerrar la biblioteca de sockets de Windows
    WSACleanup();

    return 0;
}
