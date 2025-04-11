#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>

int main() {
    const char *ip = "8.8.8.8";  // Google DNS
    struct sockaddr_in sa;
    char host[1024];

    memset(&sa, 0, sizeof sa);
    sa.sin_family = AF_INET;
    inet_pton(AF_INET, ip, &sa.sin_addr);

    int result = getnameinfo((struct sockaddr*)&sa, sizeof sa,
                             host, sizeof host,
                             NULL, 0, NI_NAMEREQD);

    if (result != 0) {
        fprintf(stderr, "getnameinfo: %s\n", gai_strerror(result));
    } else {
        printf("Host name: %s\n", host);
    }

    return 0;
}
