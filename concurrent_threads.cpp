#include <iostream>
#include <pthread.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <cstdlib>

struct thread_config_t {
    int sockfd;
};

void* server_thread(void* arg) {
    thread_config_t* config = (thread_config_t*)arg;
    int sockfd = config->sockfd;
    free(config);

    unsigned long id = (unsigned long)pthread_self();
    std::cout << "Thread " << id << " created to handle connection with socket " << sockfd << std::endl;

    char buffer[1024];
    ssize_t n;
    while ((n = read(sockfd, buffer, sizeof(buffer) - 1)) > 0) {
        buffer[n] = '\0';
        std::cout << "Thread " << id << " received: " << buffer << std::endl;

        // Echo back the received message
        write(sockfd, buffer, n);
    }

    std::cout << "Thread " << id << " done" << std::endl;
    close(sockfd);
    return nullptr;
}

int main() {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_addr;
    bzero((char*)&server_addr, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(8080);

    if (bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error on binding");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    listen(sockfd, 5);

    while (true) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        int newsockfd = accept(sockfd, (struct sockaddr*)&client_addr, &client_len);

        if (newsockfd < 0) {
            perror("Error on accept");
            continue;
        }

        thread_config_t* config = (thread_config_t*)malloc(sizeof(*config));
        if (!config) {
            std::cerr << "Out of memory" << std::endl;
            close(newsockfd);
            continue;
        }

        config->sockfd = newsockfd;

        pthread_t thread;
        pthread_create(&thread, nullptr, server_thread, config);
        pthread_detach(thread);
    }

    close(sockfd);
    return 0;
}
