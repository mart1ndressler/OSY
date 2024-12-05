#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <cstdlib>
#include <fcntl.h>
#include <mqueue.h>
#include <arpa/inet.h>

#define LIMIT 100
#define QUEUE_NAME_DONORS "/donors"
#define QUEUE_NAME_SECURITY "/security"

using namespace std;

struct Pokladnicka {
    int suma;
    char mince[1024];
};

void handle_client(int client_socket, mqd_t mq_donors, mqd_t mq_security, int port) {
    if (port % 2 == 1) { // Donors
        while (true) {
            char buffer[1024] = {0};
            int bytes_read = read(client_socket, buffer, sizeof(buffer));
            if (bytes_read <= 0) break;
            if (strcmp(buffer, "END") == 0) break;

            Pokladnicka pokladnicka;
            if (mq_receive(mq_donors, (char *)&pokladnicka, sizeof(Pokladnicka), nullptr) == -1) {
                perror("[SERVER] Failed to receive from donors queue");
                continue;
            }

            char *token = strtok(buffer, " ");
            while (token != nullptr) {
                int coin = atoi(token);
                if (coin != 1 && coin != 2 && coin != 5 && coin != 10 && coin != 20 && coin != 50) {
                    string invalid_msg = "Invalid coin value! Please send 1, 2, 5, 10, 20, or 50!\n";
                    send(client_socket, invalid_msg.c_str(), invalid_msg.size(), 0);
                    token = strtok(nullptr, " ");
                    continue;
                }

                pokladnicka.suma += coin;
                strcat(pokladnicka.mince, token);
                strcat(pokladnicka.mince, " ");

                if (pokladnicka.suma >= LIMIT) {
                    if (mq_send(mq_security, (const char *)&pokladnicka, sizeof(Pokladnicka), 0) == -1) {
                        perror("[SERVER] Failed to send to security queue");
                    } else {
                        cout << "[SERVER] Cashbox sent to security: " << pokladnicka.mince
                             << " (sum: " << pokladnicka.suma << ")" << endl;
                    }
                    pokladnicka.suma = 0;
                    memset(pokladnicka.mince, 0, sizeof(pokladnicka.mince));
                }
                token = strtok(nullptr, " ");
            }

            if (mq_send(mq_donors, (const char *)&pokladnicka, sizeof(Pokladnicka), 0) == -1) {
                perror("[SERVER] Failed to send back to donors queue");
            }

            string feedback = "Current cashbox sum: " + to_string(pokladnicka.suma) + "\n";
            send(client_socket, feedback.c_str(), feedback.size(), 0);
        }
    } else { // Security
        while (true) {
            Pokladnicka pokladnicka;
            if (mq_receive(mq_security, (char *)&pokladnicka, sizeof(Pokladnicka), nullptr) == -1) {
                perror("[SERVER] Failed to receive from security queue");
                continue;
            }

            cout << "[SECURITY] Transporting cashbox: " << pokladnicka.mince
                 << " (sum: " << pokladnicka.suma << ")" << endl;

            pokladnicka.suma = 0;
            memset(pokladnicka.mince, 0, sizeof(pokladnicka.mince));
            if (mq_send(mq_donors, (const char *)&pokladnicka, sizeof(Pokladnicka), 0) == -1) {
                perror("[SERVER] Failed to send empty cashbox back to donors queue");
            }
        }
    }
    close(client_socket);
}

int main(int argc, char *argv[]) {
    if (argc != 2) return 1;
    int port = atoi(argv[1]);

    struct mq_attr attr = {0, 10, sizeof(Pokladnicka), 0};
    mqd_t mq_donors = mq_open(QUEUE_NAME_DONORS, O_CREAT | O_RDWR, 0666, &attr);
    mqd_t mq_security = mq_open(QUEUE_NAME_SECURITY, O_CREAT | O_RDWR, 0666, &attr);
    if (mq_donors == -1 || mq_security == -1) {
        perror("[SERVER] Failed to create message queues");
        return 1;
    }

    Pokladnicka initial_pokladnicka = {0, ""};
    if (mq_send(mq_donors, (const char *)&initial_pokladnicka, sizeof(Pokladnicka), 0) == -1) {
        perror("[SERVER] Failed to initialize donors queue");
        return 1;
    }

    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == 0) {
        perror("[SERVER] Socket creation failed");
        return 1;
    }

    sockaddr_in address;
    int opt = 1, addrlen = sizeof(address);
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));
    if (bind(server_socket, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("[SERVER] Bind failed");
        return 1;
    }
    if (listen(server_socket, 5) < 0) {
        perror("[SERVER] Listen failed");
        return 1;
    }

    cout << "[SERVER] Server running on port " << port << "..." << endl;

    while (true) {
        int client_socket = accept(server_socket, (struct sockaddr *)&address, (socklen_t *)&addrlen);
        if (client_socket < 0) continue;

        if (port % 2 == 1) cout << "[SERVER] Donor client connected on port " << port << "." << endl;
        else cout << "[SERVER] Security client connected on port " << port << "." << endl;

        if (fork() == 0) {
            close(server_socket);
            handle_client(client_socket, mq_donors, mq_security, port);
            exit(0);
        } else {
            close(client_socket);
        }
    }

    mq_close(mq_donors);
    mq_close(mq_security);
    mq_unlink(QUEUE_NAME_DONORS);
    mq_unlink(QUEUE_NAME_SECURITY);
    close(server_socket);
    return 0;
}