#include <stdbool.h>
#include <stdio.h>

void HEADER_NAME_STATE(char **ptr_ptr_http_client_buffer, int new_connection_fd,
                       bool host_header_present, char *ptr_uri,
                       char *ptr_method);
void HEADER_VALUE_STATE(char **ptr_ptr_http_client_buffer,
                        int new_connection_fd, bool host_header_present,
                        char *ptr_uri, char *ptr_method);
void ERROR_STATE_400(int new_connection_fd);
void ERROR_STATE_404(int new_connection_fd);
void REQUEST_LINE_STATE(char **ptr_ptr_http_client_buffer,
                        int new_connection_fd);
void END_OF_HEADERS_STATE(int new_connection_fd, char *ptr_uri,
                          char *ptr_method);
void STATE_PARSER(char *ptr_http_client_buffer, int new_connection_fd);
void parse_HTTP_requests(int new_connection_fd);
char *receive_HTTP_request(int new_connection_fd);
void send_http_response(int new_connection_fd, char *ptr_packet_buffer);
size_t get_size_of_file(FILE *fp);
char *get_file_type_from_uri(char *ptr_uri_buffer);
void send_requested_file_back(int new_connection_fd, char *ptr_uri_buffer);
void send_requested_HEAD_back(int new_connection_fd, char *ptr_uri_buffer);
