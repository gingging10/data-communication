# data-communication task1

//Receiver.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define MAX_BUFFER_SIZE 1024
#define PORT 8080

int main() {
    int sockfd;
    struct sockaddr_in servaddr, cliaddr;
    socklen_t len = sizeof(cliaddr);

    // UDP 소켓을 생성한다.
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd == -1) {
        perror("UDP 소켓 생성 실패");
        exit(EXIT_FAILURE);
    }

    // 서버 주소 설정()
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(PORT);

    // 주소에 소켓 바인딩
    if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) == -1) {
        perror("Binding failed");
        exit(EXIT_FAILURE);
    }

    FILE *fp = NULL;
    char buffer[MAX_BUFFER_SIZE];
    ssize_t bytes_received;
    int file_opened = 0;

    while (1) {
        
        bytes_received = recvfrom(sockfd,buffer, MAX_BUFFER_SIZE, 0, (struct sockaddr *)&cliaddr, &len);
        if (bytes_received <= 0) {
            perror("데이터 수신 실패");
            continue;
        }

        /
        if (!file_opened) {

            char filename[MAX_BUFFER_SIZE];
            sprintf(filename, "received.txt");

            fp = fopen(filename, "wb");

            if (fp == NULL) {
                perror("파일 오픈 실패함.");
                continue;
            }

            file_opened = 1;
        }

       
        fwrite(buffer, 1, bytes_received, fp);

        
        if (strcmp(buffer, "WellDone") == 0) {
            printf("File transfer completed.\n");
            break;
        }
    }

    // 파일 닫기
    if (fp != NULL) {
        fclose(fp);
    }

    

    // 소켓 닫기
    close(sockfd);

    return 0;
}

## Sender.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define MAX_BUFFER_SIZE 1024

#define PORT 8080
// 함수 선언
int cre_udp_soc();
void server_address(struct sockaddr_in *server_addr);
void send_filename(int sockfd, const struct sockaddr_in *server_addr, const char *filename);
void send_to_file(int sockfd, const struct sockaddr_in *server_addr, const char *filename);

int main() {
    int sockfd;
    struct sockaddr_in server_addr;
    char buffer[MAX_BUFFER_SIZE];
    char *filename = "example.txt"; // file name

    sockfd = cre_udp_soc();

    server_address(&server_addr);

    send_filename(sockfd,&server_addr,filename);

    send_to_file(sockfd,&server_addr,filename);

    close(sockfd);

    return 0;
}

// UDP 소켓 생성 함수
int cre_udp_soc() {
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd == -1) {
        perror("UDP 소켓 생성에 실패했습니다.");
        exit(EXIT_FAILURE);
    }
    return sockfd;
}

// 서버 주소 설정 함수
void server_address(struct sockaddr_in *server_addr) {
    memset(server_addr, 0, sizeof(*server_addr));
    server_addr->sin_family = AF_INET;
    server_addr->sin_port = htons(PORT);
    server_addr->sin_addr.s_addr = INADDR_ANY;
}

// 파일 이름 전송 
void send_filename(int sockfd, const struct sockaddr_in *server_addr, const char *filename) {
    char buffer[MAX_BUFFER_SIZE];

    strcpy(buffer, "Greeting");
    sendto(sockfd, (const char *)buffer, strlen(buffer), 0, (const struct sockaddr *)server_addr, sizeof(*server_addr));
    strcpy(buffer, filename);
    sendto(sockfd, (const char *)buffer, strlen(buffer), 0, (const struct sockaddr *)server_addr, sizeof(*server_addr));
}

// 파일을 전송하는 함수
void send_to_file(int sockfd, const struct sockaddr_in *server_addr, const char *filename) {

    char buffer[MAX_BUFFER_SIZE];
    FILE *fp = fopen(filename, "rb");
    if (fp == NULL) {
        perror("File opening failed");
        exit(EXIT_FAILURE);
    }

    
    while (1) {

        size_t bytes_read = fread(buffer, 1, MAX_BUFFER_SIZE, fp);

        if (bytes_read <= 0) break;

        // 재전송
        int attempts = 0;

        while (1) {

            if (sendto(sockfd, buffer, bytes_read,0,(const struct sockaddr *)server_addr, sizeof(*server_addr)) == -1) {

                perror("데이터 전송 실패 메세지");
                attempts++;
                if (attempts >= 5) {
                    fprintf(stderr, "5번 시도 후 데이터를 보내지 못함. 중단\n");
                    exit(EXIT_FAILURE);
                }
                continue;
            }
            break;
        }
    }

    // 파일 전송 완료 메시지 전송하기
    strcpy(buffer, "EOF");
    sendto(sockfd, (const char *)buffer, strlen(buffer), 0, (const struct sockaddr *)server_addr, sizeof(*server_addr));

    fclose(fp);
}

