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

    // Crear un socket del servidor
    SOCKET server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == INVALID_SOCKET) {
        perror("Error al crear el socket del servidor");
        WSACleanup();
        exit(EXIT_FAILURE);
    }

    // Definir la dirección del servidor
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(8080);
    server_address.sin_addr.s_addr = INADDR_ANY;

    // Vincular el socket a la dirección
    if (bind(server_socket, (struct sockaddr*)&server_address, sizeof(server_address)) == SOCKET_ERROR) {
        perror("Error en la vinculación");
        closesocket(server_socket);
        WSACleanup();
        exit(EXIT_FAILURE);
    }

    // Escuchar en el socket
    if (listen(server_socket, 5) == SOCKET_ERROR) {
        perror("Error al escuchar");
        closesocket(server_socket);
        WSACleanup();
        exit(EXIT_FAILURE);
    }

    printf("El servidor está escuchando...\n");

    // Aceptar la conexión del cliente
    SOCKET client_socket = accept(server_socket, NULL, NULL);
    if (client_socket == INVALID_SOCKET) {
        perror("Error al aceptar la conexión");
        closesocket(server_socket);
        WSACleanup();
        exit(EXIT_FAILURE);
    }

    // Mensaje de bienvenida
    const char* welcome_message = "Bienvenido al servidor FTP. Puedes comenzar a chatear.\n";
    send(client_socket, welcome_message, strlen(welcome_message), 0);

    // Bucle de chat
    char buffer[256];
    while (1) {
        memset(buffer, 0, sizeof(buffer));
        int bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
        if (bytes_received <= 0) {
            break;
        }

        // Mostrar el mensaje recibido
        printf("Cliente: %s", buffer);

        // Enviar una respuesta al cliente
        printf("Servidor: ");
        fgets(buffer, sizeof(buffer), stdin);
        send(client_socket, buffer, strlen(buffer), 0);
    }

    // Cerrar los sockets
    closesocket(client_socket);
    closesocket(server_socket);
    WSACleanup();

    return 0;
}
