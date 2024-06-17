#include "packet.h"

int sockfd;
struct sockaddr_in receiver_addr;
socklen_t receiver_addr_len = sizeof(receiver_addr);
int timeout_interval;
float ack_drop_prob;

void handle_timeout(int sig) {
    
}

void send_file(const char *filename) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        perror("Failed to open file");
        exit(EXIT_FAILURE);
    }

    int seq_num = 0;
    char buffer[sizeof(Packet)];
    Packet packet;
    size_t bytes_read;

    signal(SIGALRM, handle_timeout);

    while ((bytes_read = fread(packet.payload, 1, MAX_PAYLOAD_SIZE, file)) > 0) {
        packet.type = DATA;
        packet.seq_num = seq_num;
        packet.ack_num = 0;
        packet.length = bytes_read;

        serialize_packet(&packet, buffer);
        sendto(sockfd, buffer, sizeof(Packet), 0, (struct sockaddr *)&receiver_addr, receiver_addr_len);
        log_event("Sent DATA", &packet);

        alarm(timeout_interval);

        Packet ack_packet;
        while (1) {
            if (recvfrom(sockfd, buffer, sizeof(Packet), 0, (struct sockaddr *)&receiver_addr, &receiver_addr_len) < 0) {
                if (errno == EINTR) {
                    // Timeout occurred, retransmit
                    sendto(sockfd, buffer, sizeof(Packet), 0, (struct sockaddr *)&receiver_addr, receiver_addr_len);
                    log_event("Resent DATA", &packet);
                    alarm(timeout_interval);
                }
            } else {
                deserialize_packet(buffer, &ack_packet);
                if (ack_packet.type == ACK && ack_packet.ack_num == seq_num) {
                    log_event("Received ACK", &ack_packet);
                    break;
                }
            }
        }

        alarm(0);
        seq_num++;
    }

    
    packet.type = EOT; 
    packet.seq_num = seq_num;
    packet.ack_num = 0;
    packet.length = 0;

  
    serialize_packet(&packet, buffer);
    sendto(sockfd, buffer, sizeof(Packet), 0, (struct sockaddr *)&receiver_addr, receiver_addr_len);
    log_event("Sent EOT", &packet);
    fclose(file);
}

int main(int argc, char *argv[]) {
    if (argc != 7) {
        fprintf(stderr, "Usage: %s <sender_port> <receiver_ip> <receiver_port> <timeout_interval> <filename> <ack_drop_prob>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int sender_port = atoi(argv[1]);
    char *receiver_ip = argv[2];
    int receiver_port = atoi(argv[3]);
    timeout_interval = atoi(argv[4]);
    char *filename = argv[5];
    ack_drop_prob = atof(argv[6]);

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&receiver_addr, 0, sizeof(receiver_addr));
    receiver_addr.sin_family = AF_INET;
    receiver_addr.sin_port = htons(receiver_port);
    inet_pton(AF_INET, receiver_ip, &receiver_addr.sin_addr);

    send_file(filename);

    close(sockfd);
    return 0;
}
