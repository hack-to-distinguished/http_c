#include "http.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#define BUFFER_SIZE 4096

const mime_type mime_types[] = {
    {"txt", "text/plain", "text"},       {"html", "text/html", "text"},
    {"css", "text/css", "text"},         {"csv", "text/csv", "text"},
    {"js", "text/javascript", "text"},   {"xml", "text/xml", "text"},
    {"jpg", "image/jpeg", "image"},      {"png", "image/png", "image"},
    {"gif", "image/gif", "image"},       {"webp", "image/webp", "image"},
    {"svg", "image/svg+xml", "image"},   {"ico", "image/x-icon", "image"},
    {"webm", "video/webm", "video"},     {"mp4", "video/mp4", "video"},
    {"mpg", "video/mpeg", "video"},      {"mov", "video/quicktime", "video"},
    {"qt", "video/quicktime", "video"},  {"wmv", "video/x-ms-wmv", "video"},
    {"avi", "video/x-msvideo", "video"}, {"flv", "video/x-flv", "video"}};

const size_t mime_types_len = sizeof(mime_types) / sizeof(mime_types[0]);

char *receive_HTTP_request(int new_connection_fd) {
    char *ptr_http_request_buffer = malloc(BUFFER_SIZE + 1);

    int total_received = 0;
    int bytes_recv;

    while ((bytes_recv = recv(new_connection_fd,
                              ptr_http_request_buffer + total_received,
                              BUFFER_SIZE - total_received, 0)) > 0) {
        total_received += bytes_recv;

        if (strstr(ptr_http_request_buffer, "\r\n\r\n")) {
            break;
        }

        if (total_received >= BUFFER_SIZE) {
            fprintf(stderr, "\nrequest too large");
            free(ptr_http_request_buffer);
            return NULL;
        }
    }

    if (bytes_recv == -1) {
        perror("recv failed");
        free(ptr_http_request_buffer);
        return NULL;
    }

    if (bytes_recv == 0 && total_received == 0) {
        fprintf(stderr, "\nclient disconnected");
        free(ptr_http_request_buffer);
        return NULL;
    }

    // printf("\nBytes Received: %d\nMessage Received: \n%s", total_received,
    //        ptr_http_request_buffer);
    ptr_http_request_buffer[total_received] = '\0';
    return ptr_http_request_buffer;
}

void send_http_response(int new_connection_fd, char *ptr_packet_buffer) {
    send(new_connection_fd, ptr_packet_buffer, strlen(ptr_packet_buffer), 0);
    free(ptr_packet_buffer);
    return;
}

void ERROR_STATE_400(int new_connection_fd) {
    char *ptr_packet_buffer = malloc(BUFFER_SIZE);
    char *ptr_body;
    int body_len;
    ptr_body = "<body>\r\n"
               "Error 400! Bad Request\r\n"
               "</body>\r\n";
    body_len = strlen(ptr_body);
    snprintf(ptr_packet_buffer, BUFFER_SIZE,
             "HTTP/1.1 400 Bad Request\r\n"
             "Content-Length: %d\r\n"
             "Content-Type: text/html;\r\nConnection: close\r\n\r\n"
             "%s",
             body_len, ptr_body);
    send_http_response(new_connection_fd, ptr_packet_buffer);
    return;
}

void ERROR_STATE_404(int new_connection_fd) {
    char *ptr_packet_buffer = malloc(BUFFER_SIZE);
    char *ptr_body;
    size_t body_len;
    ptr_body = "<body>\r\n"
               "Error 404! Not Found\r\n"
               "</body>\r\n";
    body_len = strlen(ptr_body);
    snprintf(ptr_packet_buffer, BUFFER_SIZE,
             "HTTP/1.1 404 Not Found\r\n"
             "Content-Length: %ld\r\n"
             "Content-Type: text/html;\r\nConnection: close\r\n\r\n"
             "%s",
             body_len, ptr_body);
    send_http_response(new_connection_fd, ptr_packet_buffer);
    return;
}
void HEADER_VALUE_STATE(char **ptr_ptr_http_client_buffer,
                        int new_connection_fd, bool host_header_present,
                        char *ptr_uri, char *ptr_method) {
    bool header_value_found = false;
    bool single_crlf_found = false;
    char *buffer = *ptr_ptr_http_client_buffer;
    char header_value[256];
    size_t header_value_counter = 0;
    size_t start_pos = 0;
    size_t buffer_len = strlen(buffer);

    // this for loop is used to skip redundant characters
    for (size_t i = 0; i < buffer_len; i++) {
        if (buffer[i] == ':' || buffer[i] == ' ' || buffer[i] == '\r' ||
            buffer[i] == '\n') {
            start_pos += 1;
        } else {
            i = buffer_len;
        }
    }

    for (NULL; start_pos < buffer_len; start_pos++) {
        if (!(buffer[start_pos] == '\r' || buffer[start_pos] == '\n') &&
            !header_value_found) {
            header_value[header_value_counter] = buffer[start_pos];
            header_value_counter += 1;
        } else {
            header_value[header_value_counter] = '\0';
            header_value_found = true;
        }

        if (start_pos < (buffer_len - 1)) {
            if (buffer[start_pos] == '\r' && buffer[start_pos + 1] == '\n' &&
                !single_crlf_found) {
                single_crlf_found = true;
                *ptr_ptr_http_client_buffer = &buffer[start_pos + 2];
            }
        }
    }

    if (single_crlf_found) {
        // printf("\nHeader Value Extracted: %s\n", header_value);
        HEADER_NAME_STATE(ptr_ptr_http_client_buffer, new_connection_fd,
                          host_header_present, ptr_uri, ptr_method);
        return;
    } else {
        printf("\nerror at header value state");
        ERROR_STATE_400(new_connection_fd);
        free(ptr_method);
        free(ptr_uri);
        return;
    }
}

size_t get_size_of_file(FILE *fp) {
    fseek(fp, 0, SEEK_END);
    size_t size_of_file = ftell(fp);
    // reset file pointer to point back to beginning of the file
    fseek(fp, 0, SEEK_SET);
    return size_of_file;
}

char *get_file_type_from_uri(char *ptr_uri_buffer) {
    // get file type
    char *file_type = malloc(sizeof(char) * 64);
    size_t counter = 0;
    bool past_period = false;
    size_t buffer_len = strlen(ptr_uri_buffer);
    for (size_t i = 0; i < buffer_len; i++) {

        if (ptr_uri_buffer[i] == '.') {
            past_period = true;
            i += 1;
        }

        if (past_period && ptr_uri_buffer[i] != '\0') {
            file_type[counter] = ptr_uri_buffer[i];
            counter += 1;
        }
    }

    file_type[counter] = '\0';
    return file_type;
}

const mime_type *get_http_mime_type(const mime_type mime_types[],
                                    char *file_type, size_t mime_types_len) {
    for (size_t i = 0; i < mime_types_len; i++) {
        if (strcmp(file_type, mime_types[i].ptr_file_extension) == 0) {
            return &mime_types[i];
        }
    }
    return NULL;
};

void send_requested_file_back(int new_connection_fd, char *ptr_uri_buffer) {
    FILE *ptr_file;
    int counter;
    char *file_type = get_file_type_from_uri(ptr_uri_buffer);

    const mime_type *mime_type =
        get_http_mime_type(mime_types, file_type, mime_types_len);

    if (mime_type == NULL) {
        fprintf(stderr, "\t Can't get http mime type \n");
        ERROR_STATE_404(new_connection_fd);
        close(new_connection_fd);
        return;
    }

    const char *file_extension = mime_type->ptr_file_extension;
    const char *content_type = mime_type->ptr_http_content_type;
    const char *content_type_prefix = mime_type->ptr_http_content_type_prefix;

    if (strcmp(content_type_prefix, "text") == 0) {

        ptr_file = fopen(ptr_uri_buffer, "r");

        if (ptr_file == NULL) {
            fprintf(stderr, "\t Can't open file : %s\n", ptr_uri_buffer);
            ERROR_STATE_404(new_connection_fd);
            close(new_connection_fd);
            return;
        }

        size_t size = get_size_of_file(ptr_file);

        char ch;
        char *ptr_file_contents = malloc(sizeof(char) * (size + 1));
        char *ptr_packet_buffer = malloc(BUFFER_SIZE + (size + 1));
        counter = 0;
        size_t file_contents_len;

        while ((ch = fgetc(ptr_file)) != EOF) {
            ptr_file_contents[counter] = ch;
            counter += 1;
        }

        ptr_file_contents[counter] = '\0';
        fclose(ptr_file);
        file_contents_len = strlen(ptr_file_contents);

        char HTTP_format[] =
            "HTTP/1.1 200 OK\r\nContent-Length: "
            "%ld\r\nContent-Type: %s\r\nConnection: close\r\n\r\n%s";

        snprintf(ptr_packet_buffer, BUFFER_SIZE, HTTP_format, file_contents_len,
                 content_type, ptr_file_contents);

        send_http_response(new_connection_fd, ptr_packet_buffer);
        free(file_type);
        free(ptr_file_contents);
        return;
    } else if (strcmp(content_type_prefix, "image") == 0 ||
               strcmp(content_type_prefix, "video") == 0) {
        ptr_file = fopen(ptr_uri_buffer, "rb");

        if (ptr_file == NULL) {
            fprintf(stderr, "\t Can't open file : %s\n", ptr_uri_buffer);
            ERROR_STATE_404(new_connection_fd);
            close(new_connection_fd);
            return;
        }

        size_t size = get_size_of_file(ptr_file);
        unsigned char *ptr_img_file_contents =
            malloc(sizeof(unsigned char) * size);
        fread(ptr_img_file_contents, sizeof(unsigned char), size, ptr_file);

        char HTTP_format[] =
            "HTTP/1.1 200 OK\r\nContent-Length: "
            "%ld\r\nContent-Type: %s\r\nConnection: close\r\n\r\n";

        char *ptr_packet_buffer = malloc(BUFFER_SIZE + size);
        snprintf(ptr_packet_buffer, BUFFER_SIZE + size, HTTP_format, size,
                 content_type);

        send_http_response(new_connection_fd, ptr_packet_buffer);
        send(new_connection_fd, ptr_img_file_contents, size, 0);
        free(ptr_img_file_contents);
        free(file_type);
        fclose(ptr_file);
        return;
    }
    return;
}

char *format_date(char *str, time_t val) {
    strftime(str, 64, "%a, %d %b %Y %H:%M:%S GMT", gmtime(&val));
    return str;
}

void send_requested_HEAD_back(int new_connection_fd, char *ptr_uri_buffer) {
    struct stat file_stat;
    char *file_type = get_file_type_from_uri(ptr_uri_buffer);

    const mime_type *mime_type =
        get_http_mime_type(mime_types, file_type, mime_types_len);

    if (mime_type == NULL) {
        fprintf(stderr, "\t Can't get http mime type \n");
        ERROR_STATE_404(new_connection_fd);
        close(new_connection_fd);
        return;
    }

    if (strcmp(ptr_uri_buffer, "/") == 0) {
        char HTTP_format[] =
            "HTTP/1.1 200 OK\r\nContent-Type: %s\r\nConnection: close\r\n\r\n";
        char *ptr_packet_buffer = malloc(BUFFER_SIZE);
        snprintf(ptr_packet_buffer, BUFFER_SIZE, HTTP_format, "text/plain");
        send_http_response(new_connection_fd, ptr_packet_buffer);
        return;
    } else {

        char HTTP_format[] =
            "HTTP/1.1 200 OK\r\nContent-Type: %s\r\nLast-Modified: "
            "%s\r\nContent-length: %lu\r\nConnection: close\r\nDate: "
            "%s\r\n\r\n";

        if (stat(ptr_uri_buffer, &file_stat) < 0) {
            ERROR_STATE_404(new_connection_fd);
            return;
        }

        char last_modified_date[64];
        char date_now[64];
        char *ptr_packet_buffer = malloc(BUFFER_SIZE);
        const char *content_type = mime_type->ptr_http_content_type;

        snprintf(ptr_packet_buffer, BUFFER_SIZE, HTTP_format, content_type,
                 format_date(last_modified_date, file_stat.st_mtime),
                 file_stat.st_size, format_date(date_now, time(NULL)));
        send_http_response(new_connection_fd, ptr_packet_buffer);
        return;
    }
}

void parse_body_of_POST(int new_connection_fd) { return; }

void END_OF_HEADERS_STATE(int new_connection_fd, char *ptr_uri,
                          char *ptr_method) {

    char *processed_uri_ptr = ptr_uri;

    if (!(strcmp(ptr_uri, "/") == 0)) {
        processed_uri_ptr += 1;
    }

    char *uri_buffer = strdup(processed_uri_ptr);
    char *ptr_uri_buffer = uri_buffer;
    FILE *file_ptr = fopen(uri_buffer, "r");

    if (file_ptr == NULL) {
        fprintf(stderr, "\t Can't open file : %s\n", ptr_uri_buffer);
        ERROR_STATE_404(new_connection_fd);
        close(new_connection_fd);
        return;
    }

    struct stat sb;
    stat(uri_buffer, &sb);

    if (access(uri_buffer, F_OK) == 0 && !S_ISDIR(sb.st_mode) &&
        strcmp(ptr_method, "GET") == 0) {
        send_requested_file_back(new_connection_fd, ptr_uri_buffer);
        free(uri_buffer);
        free(ptr_uri);
        free(ptr_method);
        fclose(file_ptr);
        return;
    } else if (strcmp(ptr_method, "HEAD") == 0 &&
               access(uri_buffer, F_OK) == 0 && !S_ISDIR(sb.st_mode)) {
        // printf("\nhead request");
        send_requested_HEAD_back(new_connection_fd, ptr_uri_buffer);
        free(uri_buffer);
        free(ptr_uri);
        free(ptr_method);
        return;
    } else if (strcmp(ptr_method, "POST") == 0) {
        printf("\nPOST Method detected!");
        parse_body_of_POST(new_connection_fd);
        free(uri_buffer);
        free(ptr_uri);
        free(ptr_method);
        return;
    } else {
        // printf("\nFile does not exist!");
        ERROR_STATE_404(new_connection_fd);
        free(uri_buffer);
        free(ptr_uri);
        free(ptr_method);
        return;
    }
}

void HEADER_NAME_STATE(char **ptr_ptr_http_client_buffer, int new_connection_fd,
                       bool host_header_present, char *ptr_uri,
                       char *ptr_method) {
    // printf("\nreached header name state");
    char *buffer = *ptr_ptr_http_client_buffer;
    char header_name[256];
    bool colon_found = false;
    bool single_crlf_found = false;
    size_t counter = 0;
    size_t buffer_len = strlen(buffer);

    // printf("\n size of buffer: %ld", buffer_len);
    // extract the header name from the header field
    for (size_t i = 0; i < buffer_len; i++) {
        if (buffer[i] == ':') {
            colon_found = true;
            *ptr_ptr_http_client_buffer = &buffer[i];
            i = buffer_len;
        }

        if (!single_crlf_found) {
            header_name[counter] = buffer[i];
            counter += 1;
        } else {
            header_name[counter] = '\0';
        }

        if (buffer_len == 2 && buffer[i] == '\r' && buffer[i + 1] == '\n') {
            single_crlf_found = true;
            // no need to check buffer len = 2 if method is POST, as it has a
            // body unlike HEAD and GET
        } else if (strcmp(ptr_method, "POST") == 0 && buffer[i] == '\r' &&
                   buffer[i + 1] == '\n') {
            single_crlf_found = true;
        }
    }

    if (single_crlf_found && host_header_present) {
        // printf("\nReached end of headers state!");
        END_OF_HEADERS_STATE(new_connection_fd, ptr_uri, ptr_method);
        return;
    }

    if (colon_found) {
        // printf("\nHeader Name Extracted: %s", header_name);
        if (strcmp(header_name, "Host") == 0) {
            host_header_present = true;
        }
        HEADER_VALUE_STATE(ptr_ptr_http_client_buffer, new_connection_fd,
                           host_header_present, ptr_uri, ptr_method);
        return;
    } else {
        printf("\nerror at header name state");
        ERROR_STATE_400(new_connection_fd);
        free(ptr_method);
        free(ptr_uri);
        return;
    }
}

void REQUEST_LINE_STATE(char **ptr_ptr_http_client_buffer,
                        int new_connection_fd) {
    char *buffer =
        *ptr_ptr_http_client_buffer; // dereference the pointer pointer
                                     // to get the actual char buffer
    char *ptr_method = malloc(sizeof(char) * 8);
    char *ptr_uri = malloc(sizeof(char) * 1025);
    char http_version[16];
    bool host_header_present = false;
    int result = sscanf(buffer, "%s %s %s", ptr_method, ptr_uri, http_version);

    char *crlf_ptr = strstr(buffer, http_version);
    if (crlf_ptr == NULL) {
        ERROR_STATE_400(new_connection_fd);
        printf("\nerror at request line state");
        free(ptr_method);
        free(ptr_uri);
        return;
    }
    crlf_ptr += 8;
    if (result != 3) {
        ERROR_STATE_400(new_connection_fd);
        printf("\nerror at request line state");
        free(ptr_method);
        free(ptr_uri);
        return;
    }

    if (!(strcmp(ptr_method, "GET") == 0 || strcmp(ptr_method, "POST") == 0 ||
          strcmp(ptr_method, "HEAD") == 0)) {
        ERROR_STATE_400(new_connection_fd);
        printf("\nerror at request line state");
        free(ptr_method);
        free(ptr_uri);
        return;
    }

    if (strcmp(http_version, "HTTP/1.1") != 0) {
        ERROR_STATE_400(new_connection_fd);
        printf("\nerror at request line state");
        free(ptr_method);
        free(ptr_uri);
        return;
    }

    if (!(crlf_ptr[0] == '\r' && crlf_ptr[1] == '\n')) {
        ERROR_STATE_400(new_connection_fd);
        printf("\nerror at request line state");
        free(ptr_method);
        free(ptr_uri);
        return;
    }

    size_t len_method = strlen(ptr_method);
    size_t len_uri = strlen(ptr_uri);
    ptr_uri[len_uri] = '\0';
    ptr_method[len_method] = '\0';

    if (!(buffer[len_method - 1] != ' ' && buffer[len_method] == ' ' &&
          buffer[len_method + 1] == '/' &&
          buffer[len_method + len_uri + 1] == ' ' &&
          buffer[len_method + len_uri + 2] != ' ')) {
        ERROR_STATE_400(new_connection_fd);
        printf("\nerror at request line state");
        free(ptr_method);
        free(ptr_uri);
        return;
    }

    crlf_ptr += 2;
    ptr_ptr_http_client_buffer = &crlf_ptr;
    //
    // printf("\nHTTP Method: %s", ptr_method);
    // printf("\nURI: %s", ptr_uri);
    // printf("\nHTTP Version: %s\n", http_version);
    //
    HEADER_NAME_STATE(ptr_ptr_http_client_buffer, new_connection_fd,
                      host_header_present, ptr_uri, ptr_method);
    return;
}

void STATE_PARSER(char *ptr_http_client_buffer, int new_connection_fd) {
    REQUEST_LINE_STATE(&ptr_http_client_buffer, new_connection_fd);
    return;
}

void parse_HTTP_requests(int new_connection_fd) {
    char *ptr_http_client_buffer = receive_HTTP_request(new_connection_fd);
    if (ptr_http_client_buffer == NULL) {
        free(ptr_http_client_buffer);
        return;
    }

    STATE_PARSER(ptr_http_client_buffer, new_connection_fd);

    free(ptr_http_client_buffer);
    return;
}
