#include <stdbool.h>
#include <stdio.h>
#include <time.h>

typedef struct {
    const char *ptr_file_extension;
    const char *ptr_http_content_type;
    const char *ptr_http_content_type_prefix;
} mime_type;

typedef struct {
    char **ptr_ptr_http_client_buffer;
    int new_connection_fd;
    bool host_header_present;
    char *ptr_uri;
    char *ptr_method;
    char *ptr_body_content_type;
} http_request_ctx;

extern const mime_type mime_types[];
extern const size_t mime_types_len;

// TODO: CREATE TYPEDEF STRUCT TO CLEAN UP ALL THESE UGLY AF POINTERS
void HEADER_NAME_STATE(http_request_ctx *ctx);
void HEADER_VALUE_STATE(http_request_ctx *ctx);
void ERROR_STATE_400(http_request_ctx *ctx);
void ERROR_STATE_404(http_request_ctx *ctx);
void REQUEST_LINE_STATE(http_request_ctx *ctx);
void END_OF_HEADERS_STATE(http_request_ctx *ctx);
void parse_HTTP_requests(int new_connection_fd);
char *receive_HTTP_request(int new_connection_fd);
void send_http_response(int new_connection_fd, char *ptr_packet_buffer);
size_t get_size_of_file(FILE *fp);
char *get_file_type_from_uri(char *ptr_uri_buffer);
const mime_type *get_http_mime_type(const mime_type mime_types[],
                                    char *file_type, size_t mime_types_len);
void send_requested_file_back(http_request_ctx *ctx, char *ptr_uri_buffer);
void send_requested_HEAD_back(http_request_ctx *ctx, char *ptr_uri_buffer);
void parse_body_of_POST(http_request_ctx *ctx);
void process_body_of_POST(http_request_ctx *ctx);
char *format_date(char *str, time_t val);
