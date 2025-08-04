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

#define BUFFER_SIZE 1024

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
    size_t body_len = 0;
    bool header_read = false;
    int i = 0;

    while ((bytes_recv = recv(new_connection_fd,
                              ptr_http_request_buffer + total_received,
                              BUFFER_SIZE, 0)) > 0) {
        total_received += bytes_recv;

        // printf("\nTotal Recv: %d bytes at pointer %p", total_received,
        //        ptr_http_request_buffer);

        if (total_received >= BUFFER_SIZE - 1) {
            printf("\nBuffer Size not large enough, dynamically resizing...");
            char *new_ptr =
                realloc(ptr_http_request_buffer, total_received + BUFFER_SIZE);
            ptr_http_request_buffer = new_ptr;
        }

        char *end_of_headers = strstr(ptr_http_request_buffer, "\r\n\r\n");
        if (end_of_headers != NULL) {
            header_read = true;
            char *result = memmem(ptr_http_request_buffer, BUFFER_SIZE + 1,
                                  "Content-Length: ", 15);
            if (result != NULL) {
                body_len = atoi(result + 15);
            }
        }

        if (header_read) {
            size_t header_size = (end_of_headers + 4) - ptr_http_request_buffer;
            size_t body_bytes = total_received - header_size;
            if (body_bytes >= body_len) {
                break;
            }
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

    ptr_http_request_buffer[total_received] = '\0';
    // printf("\nBytes Received: %d\nMessage Received: \n%s", total_received,
    //        ptr_http_request_buffer);
    return ptr_http_request_buffer;
}

void send_http_response(int new_connection_fd, char *ptr_packet_buffer) {
    send(new_connection_fd, ptr_packet_buffer, strlen(ptr_packet_buffer), 0);
    free(ptr_packet_buffer);
    return;
}

void ERROR_STATE_400(http_request_ctx *ctx) {
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
    send_http_response(ctx->new_connection_fd, ptr_packet_buffer);
    return;
}

void ERROR_STATE_404(http_request_ctx *ctx) {
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
    send_http_response(ctx->new_connection_fd, ptr_packet_buffer);
    return;
}

void HEADER_VALUE_STATE(http_request_ctx *ctx) {
    bool header_value_found = false;
    bool single_crlf_found = false;
    char *buffer = *ctx->ptr_ptr_http_client_buffer;
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
                *ctx->ptr_ptr_http_client_buffer = &buffer[start_pos + 2];
            }
        }
    }

    if (single_crlf_found) {
        // printf("\nHeader Value Extracted: %s\n", header_value);
        HEADER_NAME_STATE(ctx);
        return;
    } else {
        printf("\nerror at header value state");
        ERROR_STATE_400(ctx);
        free(ctx->ptr_method);
        free(ctx->ptr_uri);
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

void send_requested_file_back(http_request_ctx *ctx, char *ptr_uri_buffer) {
    FILE *ptr_file;
    int counter;
    char *file_type = get_file_type_from_uri(ptr_uri_buffer);

    const mime_type *mime_type =
        get_http_mime_type(mime_types, file_type, mime_types_len);

    if (mime_type == NULL) {
        fprintf(stderr, "\t Can't get http mime type \n");
        ERROR_STATE_404(ctx);
        close(ctx->new_connection_fd);
        return;
    }

    const char *file_extension = mime_type->ptr_file_extension;
    const char *content_type = mime_type->ptr_http_content_type;
    const char *content_type_prefix = mime_type->ptr_http_content_type_prefix;

    if (strcmp(content_type_prefix, "text") == 0) {

        ptr_file = fopen(ptr_uri_buffer, "r");

        if (ptr_file == NULL) {
            fprintf(stderr, "\t Can't open file : %s\n", ptr_uri_buffer);
            ERROR_STATE_404(ctx);
            close(ctx->new_connection_fd);
            return;
        }

        size_t size = get_size_of_file(ptr_file);

        char ch;
        char *ptr_file_contents = malloc(sizeof(char) * (size + 1));
        char *ptr_packet_buffer = malloc(BUFFER_SIZE + size + 1);
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

        snprintf(ptr_packet_buffer, BUFFER_SIZE + size + 1, HTTP_format,
                 file_contents_len, content_type, ptr_file_contents);

        send_http_response(ctx->new_connection_fd, ptr_packet_buffer);
        free(file_type);
        free(ptr_file_contents);
        return;
    } else if (strcmp(content_type_prefix, "image") == 0 ||
               strcmp(content_type_prefix, "video") == 0) {
        ptr_file = fopen(ptr_uri_buffer, "rb");

        if (ptr_file == NULL) {
            fprintf(stderr, "\t Can't open file : %s\n", ptr_uri_buffer);
            ERROR_STATE_404(ctx);
            close(ctx->new_connection_fd);
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

        send_http_response(ctx->new_connection_fd, ptr_packet_buffer);
        send(ctx->new_connection_fd, ptr_img_file_contents, size, 0);
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

void send_requested_HEAD_back(http_request_ctx *ctx, char *ptr_uri_buffer) {
    struct stat file_stat;
    char *file_type = get_file_type_from_uri(ptr_uri_buffer);

    const mime_type *mime_type =
        get_http_mime_type(mime_types, file_type, mime_types_len);

    if (mime_type == NULL) {
        fprintf(stderr, "\t Can't get http mime type \n");
        ERROR_STATE_404(ctx);
        close(ctx->new_connection_fd);
        return;
    }

    if (strcmp(ptr_uri_buffer, "/") == 0) {
        char HTTP_format[] =
            "HTTP/1.1 200 OK\r\nContent-Type: %s\r\nConnection: close\r\n\r\n";
        char *ptr_packet_buffer = malloc(BUFFER_SIZE);
        snprintf(ptr_packet_buffer, BUFFER_SIZE, HTTP_format, "text/plain");
        send_http_response(ctx->new_connection_fd, ptr_packet_buffer);
        return;
    } else {

        char HTTP_format[] =
            "HTTP/1.1 200 OK\r\nContent-Type: %s\r\nLast-Modified: "
            "%s\r\nContent-length: %lu\r\nConnection: close\r\nDate: "
            "%s\r\n\r\n";

        if (stat(ptr_uri_buffer, &file_stat) < 0) {
            ERROR_STATE_404(ctx);
            return;
        }

        char last_modified_date[64];
        char date_now[64];
        char *ptr_packet_buffer = malloc(BUFFER_SIZE);
        const char *content_type = mime_type->ptr_http_content_type;

        snprintf(ptr_packet_buffer, BUFFER_SIZE, HTTP_format, content_type,
                 format_date(last_modified_date, file_stat.st_mtime),
                 file_stat.st_size, format_date(date_now, time(NULL)));
        send_http_response(ctx->new_connection_fd, ptr_packet_buffer);
        return;
    }
}

void parse_body_of_POST(http_request_ctx *ctx) {
    char *ptr_body =
        malloc(sizeof(char) * (((int)*ctx->ptr_body_content_length) + 1));
    strncpy(ptr_body, *ctx->ptr_ptr_http_client_buffer,
            (int)*ctx->ptr_body_content_length + 1);
    ptr_body[(int)*ctx->ptr_body_content_length] = '\0';

    // printf("\nBody: %s", ptr_body);
    // printf("\nContent-Type of Body of Request: %s",
    // ctx->ptr_body_content_type); printf("\nContent-Length of Body of Request:
    // %zu ",
    //        *ctx->ptr_body_content_length);

    char HTTP_format[] = "HTTP/1.1 200 OK\r\nContent-Type: "
                         "text/html\r\nConnection: close\r\n\r\n%s";
    char *ptr_packet_buffer = malloc(BUFFER_SIZE);
    snprintf(ptr_packet_buffer, BUFFER_SIZE, HTTP_format, ptr_body);
    send_http_response(ctx->new_connection_fd, ptr_packet_buffer);

    free(ptr_body);
    free(ctx->ptr_body_content_type);
    free(ctx->ptr_body_content_length);
    return;
}

void END_OF_HEADERS_STATE(http_request_ctx *ctx) {

    char *processed_uri_ptr = ctx->ptr_uri;

    if (!(strcmp(ctx->ptr_uri, "/") == 0)) {
        processed_uri_ptr += 1;
    }

    char *uri_buffer = strdup(processed_uri_ptr);
    char *ptr_uri_buffer = uri_buffer;
    FILE *file_ptr = fopen(uri_buffer, "r");

    if (file_ptr == NULL && strcmp(ctx->ptr_method, "GET") == 0) {
        fprintf(stderr, "\t Can't open file : %s\n", ptr_uri_buffer);
        ERROR_STATE_404(ctx);
        close(ctx->new_connection_fd);
        return;
    }

    struct stat sb;
    stat(uri_buffer, &sb);

    if (access(uri_buffer, F_OK) == 0 && !S_ISDIR(sb.st_mode) &&
        strcmp(ctx->ptr_method, "GET") == 0) {
        // printf("\nGET");
        send_requested_file_back(ctx, ptr_uri_buffer);
        free(uri_buffer);
        free(ctx->ptr_uri);
        free(ctx->ptr_method);
        fclose(file_ptr);
        return;
    } else if (strcmp(ctx->ptr_method, "HEAD") == 0 &&
               access(uri_buffer, F_OK) == 0 && !S_ISDIR(sb.st_mode)) {
        send_requested_HEAD_back(ctx, ptr_uri_buffer);
        free(uri_buffer);
        free(ctx->ptr_uri);
        free(ctx->ptr_method);
        return;
    } else if (strcmp(ctx->ptr_method, "POST") == 0) {
        parse_body_of_POST(ctx);
        free(uri_buffer);
        free(ctx->ptr_uri);
        free(ctx->ptr_method);
        return;
    } else {
        // printf("\nFile does not exist!");
        ERROR_STATE_404(ctx);
        free(uri_buffer);
        free(ctx->ptr_uri);
        free(ctx->ptr_method);
        return;
    }
}

void HEADER_NAME_STATE(http_request_ctx *ctx) {
    // printf("\nreached header name state");
    char *buffer = *ctx->ptr_ptr_http_client_buffer;
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
            *ctx->ptr_ptr_http_client_buffer = &buffer[i];
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
            i = buffer_len;
        } else if (strcmp(ctx->ptr_method, "POST") == 0 && buffer[i] == '\r' &&
                   buffer[i + 1] == '\n') {
            single_crlf_found = true;
            *ctx->ptr_ptr_http_client_buffer = &buffer[i + 2];
            i = buffer_len;
        }
    }

    if (single_crlf_found && ctx->host_header_present) {
        // printf("\nReached end of headers state!");
        END_OF_HEADERS_STATE(ctx);
        return;
    }

    if (colon_found) {
        // printf("\nHeader Name Extracted: %s", header_name);
        if (strcmp(header_name, "Host") == 0) {
            ctx->host_header_present = true;
        }
        HEADER_VALUE_STATE(ctx);
        return;
    } else {
        printf("\nerror at header name state");
        ERROR_STATE_400(ctx);
        free(ctx->ptr_method);
        free(ctx->ptr_uri);
        return;
    }
}

void REQUEST_LINE_STATE(http_request_ctx *ctx) {
    // char *buffer = malloc(8);
    // buffer = *ctx->ptr_ptr_http_client_buffer;
    char *buffer = *ctx->ptr_ptr_http_client_buffer;

    ctx->ptr_method = malloc(sizeof(char) * 8);
    ctx->ptr_uri = malloc(sizeof(char) * 1025);
    char http_version[16];
    ctx->host_header_present = false;
    int result =
        sscanf(buffer, "%s %s %s", ctx->ptr_method, ctx->ptr_uri, http_version);

    char *ptr_body_content_type;
    char *start = strstr(buffer, "Content-Type: ");
    char *end;
    if (start != NULL) {
        start += 14;
        size_t pos = 0;
        while (start[pos] != '\r') {
            pos += 1;
            end = &start[pos];
        }
        ptr_body_content_type = malloc(sizeof(char) * ((end - start) + 1));
        strncpy(ptr_body_content_type, start, end - start);
        ptr_body_content_type[end - start] = '\0';
    }
    ctx->ptr_body_content_type = ptr_body_content_type;

    size_t *ptr_body_content_length;
    start = strstr(buffer, "Content-Length: ");
    if (start != NULL) {
        start += 16;
        size_t content_length = (size_t)atoi(start);
        size_t *temp_ptr = &content_length;
        ptr_body_content_length = malloc(sizeof(size_t));
        memcpy(ptr_body_content_length, temp_ptr, sizeof(size_t));
        // printf("\n%zu", *ptr_body_content_length);
    }
    ctx->ptr_body_content_length = ptr_body_content_length;

    char *crlf_ptr = strstr(buffer, http_version);
    if (crlf_ptr == NULL) {
        ERROR_STATE_400(ctx);
        printf("\nerror at request line state");
        free(ctx->ptr_method);
        free(ctx->ptr_uri);
        return;
    }
    crlf_ptr += 8;
    if (result != 3) {
        ERROR_STATE_400(ctx);
        printf("\nerror at request line state");
        free(ctx->ptr_method);
        free(ctx->ptr_uri);
        return;
    }

    if (!(strcmp(ctx->ptr_method, "GET") == 0 ||
          strcmp(ctx->ptr_method, "POST") == 0 ||
          strcmp(ctx->ptr_method, "HEAD") == 0)) {
        ERROR_STATE_400(ctx);
        printf("\nerror at request line state");
        free(ctx->ptr_method);
        free(ctx->ptr_uri);
        return;
    }

    if (strcmp(http_version, "HTTP/1.1") != 0) {
        ERROR_STATE_400(ctx);
        printf("\nerror at request line state");
        free(ctx->ptr_method);
        free(ctx->ptr_uri);
        return;
    }

    if (!(crlf_ptr[0] == '\r' && crlf_ptr[1] == '\n')) {
        ERROR_STATE_400(ctx);
        printf("\nerror at request line state");
        free(ctx->ptr_method);
        free(ctx->ptr_uri);
        return;
    }

    size_t len_method = strlen(ctx->ptr_method);
    size_t len_uri = strlen(ctx->ptr_uri);
    ctx->ptr_uri[len_uri] = '\0';
    ctx->ptr_method[len_method] = '\0';

    if (!(buffer[len_method - 1] != ' ' && buffer[len_method] == ' ' &&
          buffer[len_method + 1] == '/' &&
          buffer[len_method + len_uri + 1] == ' ' &&
          buffer[len_method + len_uri + 2] != ' ')) {
        ERROR_STATE_400(ctx);
        printf("\nerror at request line state");
        free(ctx->ptr_method);
        free(ctx->ptr_uri);
        return;
    }

    crlf_ptr += 2;
    ctx->ptr_ptr_http_client_buffer = &crlf_ptr;
    //
    // printf("\nHTTP Method: %s", ptr_method);
    // printf("\nURI: %s", ptr_uri);
    // printf("\nHTTP Version: %s\n", http_version);
    //
    HEADER_NAME_STATE(ctx);
    return;
}

void parse_HTTP_requests(int new_connection_fd) {
    http_request_ctx *ctx = malloc(sizeof(http_request_ctx));
    char *ptr_http_client_buffer = receive_HTTP_request(new_connection_fd);
    if (ptr_http_client_buffer == NULL) {
        free(ptr_http_client_buffer);
        return;
    }

    ctx->new_connection_fd = new_connection_fd;
    ctx->ptr_ptr_http_client_buffer = &ptr_http_client_buffer;
    REQUEST_LINE_STATE(ctx);

    free(ptr_http_client_buffer);
    free(ctx);
    return;
}
