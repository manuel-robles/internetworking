#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib")  // Enlazar con la biblioteca de Winsock

int main() {
    // Inicializar Winsock
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        perror("Error al inicializar Winsock");
        exit(EXIT_FAILURE);
    }

    // Crear un socket del cliente
    SOCKET client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == INVALID_SOCKET) {
        perror("Error al crear el socket del cliente");
        WSACleanup();
        exit(EXIT_FAILURE);
    }

    // Definir la dirección del servidor
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(8080);
    server_address.sin_addr.s_addr = inet_addr("127.0.0.1");  // Dirección IP del servidor

    // Conectar al servidor
    if (connect(client_socket, (struct sockaddr*)&server_address, sizeof(server_address)) == SOCKET_ERROR) {
        perror("Error al conectar al servidor");
        closesocket(client_socket);
        WSACleanup();
        exit(EXIT_FAILURE);
    }

    // Bucle de chat
    char buffer[256];
    while (1) {
        // Recibir mensaje del servidor
        memset(buffer, 0, sizeof(buffer));
        int bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
        if (bytes_received <= 0) {
            break;
        }

        // Mostrar el mensaje del servidor
        printf("Servidor: %s", buffer);

        // Enviar una respuesta al servidor
        printf("Cliente: ");
        fgets(buffer, sizeof(buffer), stdin);
        send(client_socket, buffer, strlen(buffer), 0);
    }

    // Cerrar el socket del cliente
    closesocket(client_socket);
    WSACleanup();

    return 0;
}
