#include "packet.h"
int sockfd;
struct sockaddr_in sender_addr;
socklen_t sender_addr_len = sizeof(sender_addr);
float data_drop_prob;

void receive_file() {
    char buffer[sizeof(Packet)];
    Packet packet;
    int expected_seq_num = 0;

    while (1) {
        if (recvfrom(sockfd, buffer, sizeof(Packet), 0, (struct sockaddr *)&sender_addr, &sender_addr_len) > 0) {
            deserialize_packet(buffer, &packet);

            if (((float)rand() / RAND_MAX) < data_drop_prob) {
                log_event("Dropped DATA", &packet);
                continue;
            }

            log_event("Received DATA", &packet);

            if (packet.type == EOT) {
                log_event("Received EOT", &packet);
                break;
            }

            if (packet.seq_num == expected_seq_num) {
               
                expected_seq_num++;
            }
            // ACK 보내기
            Packet ack_packet = {ACK, 0, packet.seq_num, 0, ""};
            serialize_packet(&ack_packet, buffer);
            sendto(sockfd, buffer, sizeof(Packet), 0, (struct sockaddr *)&sender_addr, sender_addr_len);
            log_event("Sent ACK", &ack_packet);
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <receiver_port> <data_drop_prob>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int receiver_port = atoi(argv[1]);
    data_drop_prob = atof(argv[2]);

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in receiver_addr;
    memset(&receiver_addr, 0, sizeof(receiver_addr));
    receiver_addr.sin_family = AF_INET;
    receiver_addr.sin_port = htons(receiver_port);
    receiver_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sockfd, (const struct sockaddr *)&receiver_addr, sizeof(receiver_addr)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    receive_file();

    close(sockfd);
    return 0;
}
