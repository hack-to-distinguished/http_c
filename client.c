#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#define MYPORT "8080"
#define MAX_IN 20


void error(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <server-address>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *server = argv[1];
    struct addrinfo hints, *res;
    int client_fd;
    int reuse_addr_flag = 1;

    printf("Connecting to server: %s\n", server);

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(server, MYPORT, &hints, &res) != 0) {
        error("getaddrinfo failed");
    }

    client_fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (client_fd == -1) {
        error("socket failed");
    }

    setsockopt(client_fd, SOL_SOCKET, SO_REUSEADDR, &reuse_addr_flag,
               sizeof(reuse_addr_flag));

    if (connect(client_fd, res->ai_addr, res->ai_addrlen) == -1) {
        error("connection failed");
    }

    const char *string_to_parse = argv[2];
    /*int sent_data =*/
    /*    send(client_fd, string_to_parse, strlen(string_to_parse), 0);*/
    /*if (sent_data == -1) {*/
    /*    error("send failed");*/
    /*}*/
    printf("Message sent to server: %s\n", string_to_parse);

    char buf[1024];
    while (recv(client_fd, buf, sizeof(buf), 0) > 0) {
        printf("\nMessage(s) recevied from server:\n");
        printf("%s\n", buf);
    } 
    // INFO: The while loop won't be interactive. Once the server finishes sending messages,
    // it will stop and won't run again

    /*
     * TODO: Stream messages:
     * Keep the command line active for sending more messages - DONE
     * Get the message sent and received, worry about the integrety of the message later
     * Handle the length of the string send so that the packet don't mess up
     * Handle the hex mapping (unicode, ASCII, etc)
     */

    puts("Press C-c to quit:");

    int ch[MAX_IN];
    int i = 0, n = 0, c, bytes_sent;
    /*while ((c = getchar()) != EOF) {*/
    /*    putchar(c);*/
    /*    printf("inputed: %c\n", (char)(c));*/
    /*    char *cmdl_msg = malloc(i + 1);*/
    /*    cmdl_msg[i] = c;*/
    /*    if (c == 10) {*/
    /*        printf("\nMessage to send: %s\n", cmdl_msg);*/
    /**/
    /*        int bytes_sent = send(client_fd, cmdl_msg, strlen(cmdl_msg), 0); */
    /*        if (bytes_sent == -1) {*/
    /*            error("unable to send entered messaged");*/
    /*        } else {*/
    /*            printf("Bytes sent: %d - ", bytes_sent);*/
    /*            printf("Message sent: %s\n", cmdl_msg);*/
    /*        }*/
    /*        free(cmdl_msg);*/
    /*        i = -1;*/
    /*    }*/
    /*    i++;*/
    /*}*/

    while (i < MAX_IN && (ch[i] = getchar()) != EOF) {
        // I think the problem lies with the getchar macro and how I'm using it

        if (ch[i] == 10) { // 10 is '\n'
            printf("\nNew line detected: %d\n", ch[i]);
            char *cmdl_msg = malloc(i + 1);
            for (int j = 0; j < i; j++) {
                cmdl_msg[j] = ch[j];
            }
            cmdl_msg[i] = '\0';
            printf("message to send: %s\n", cmdl_msg);
            // INFO: This is where the second message ends for some reason

            // This ends the process for some reason
            bytes_sent = send(client_fd, cmdl_msg, strlen(cmdl_msg), 0); 
            if (bytes_sent == -1) {
                error("unable to send entered messaged");
            } else {
                printf("Bytes sent: %d - ", bytes_sent);
                printf("Message sent: %s\n", cmdl_msg);
            }
            free(cmdl_msg);
            i = -1;
        }
        printf("current char: %c ", (char)(ch[i]));
        ++n;
        i++;
        printf("Space left %d/%d\n", i, MAX_IN);
    }

    /*char buffer[MAX_IN], *chr[MAX_IN];*/
    /*while (i < MAX_IN && (fgets(buffer, sizeof(buffer), stdin))) {*/
    /*    // I think the problem lies with the getchar macro and how I'm using it*/
    /*    chr[i] = strdup(buffer);*/
    /*    printf("stuff %c", chr[i]);*/
    /**/
    /*    if (chr[i] == "\n") {*/
            /*char *cmdl_msg = malloc(i + 1);*/
            /*for (int j = 0; j < i; j++) {*/
            /*    cmdl_msg[j] = chr[j];*/
            /*}*/
            /*cmdl_msg[i+1] = '\0';*/
            /*printf("message to send: %s\n", cmdl_msg);*/
    /*        // INFO: This is where the second message ends for some reason*/
    /**/
    /*        // This ends the process for some reason*/
    /**/
    /*        printf("message to send: %s\n", buffer);*/
    /*        bytes_sent = send(client_fd, buffer, strlen(buffer), 0); */
    /*        if (bytes_sent == -1) {*/
    /*            error("unable to send entered messaged");*/
    /*        } else {*/
    /*            printf("Bytes sent: %d - ", bytes_sent);*/
    /*            printf("Message sent: %s\n", buffer);*/
    /*        }*/
            /*free(cmdl_msg);*/
            /*i = 0;*/
    /*    } else {*/
    /*        ++n;*/
    /*        i++;*/
    /*    }*/
    /*}*/


    // SUMMARY
    printf("\n");
    for (i = 0; i < n; ++i) {
        printf("%c ", (char)(ch[i]));
    }
    printf("\n");

    freeaddrinfo(res);
    return 0;
}
