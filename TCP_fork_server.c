// TCP-сервер, обслуживающий входящие запросы от пользователей 
//   с параллельной обработкой запросов
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>

int main() {
    int sockfd, newsockfd;  // Дескрипторы для слушающего и присоединенного сокетов
    int clilen;             // Длина адреса клиента
    int n;                  // Количество принятых символов
    char line[1000];        // Буфер для приема информации
    struct sockaddr_in servaddr, cliaddr;   // Структуры  для размещения полных адресов сервера и клиента
    int chpid;              // PID порожденного процесса
    // Создаем TCP-сокет
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror(NULL);
        exit(1);
    }
    // Заполняем структуру для адреса сервера
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(51000);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    bzero(servaddr.sin_zero, 8);
    // Настраиваем адрес сокета
    if(bind(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0) {
        perror(NULL);
        close(sockfd);
        exit(1);
    }
    // Переводим созданный сокет в пассивное (слушающее) состояние.
    if(listen(sockfd, 5) < 0) {
        perror(NULL);
        close(sockfd);
        exit(1);
    }
    // Основной цикл сервера
    while (1) {
        // В переменную clilen заносим максимальную длину ожидаемого адреса клиента
        clilen = sizeof(cliaddr);
        // Ожидаем полностью установленного соединения на слушающем сокете
        if((newsockfd = accept(sockfd, (struct sockaddr *) &cliaddr, &clilen)) < 0) {
            perror(NULL);
            close(sockfd);
            exit(1);
        }
        // Порождаем новый процесс
        chpid = fork();
        if (chpid < 0) {
            printf("Can't fork child!\n");
        } else if (chpid == 0) {
            // Находимся в процессе-ребенке
            // В цикле принимаем информацию от клиента до тех пор, пока не произойдет ошибки или клиент не закроет соединение
            while((n = read(newsockfd, line, 999)) > 0){
                // Принятые данные отправляем обратно
                if((n = write(newsockfd, line, strlen(line)+1)) < 0){
                    perror(NULL);
                    close(sockfd);
                    close(newsockfd);
                    exit(1);
                }
            }
            // Если при чтении возникла ошибка – завершаем работу
            if (n < 0) {
                perror(NULL);
                close(sockfd);
                close(newsockfd);
                exit(1);
            }
            // Закрываем дескриптор присоединенного сокета
            close(newsockfd);
            // Выходим из порожденного процесса
            exit(1);
        } else {
            // Закрываем присоединенный сокет и уходим ждать нового соединения
            close(newsockfd);
        }
    }
    return 0;
}