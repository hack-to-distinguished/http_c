// #include <stddef.h>
// #include <stdio.h>
// #include <string.h>
#include <time.h>

#define USER_ID_SIZE 256
#define MSG_STORE_SIZE 1000

typedef struct
{
    char   sender_id[USER_ID_SIZE];
    char   recipient_id[USER_ID_SIZE];
    size_t msg_len;
    time_t send_time; // Likely only storing time here
    time_t recv_time; // Likely only storing time here
    char   msg_type;
    size_t send_status; // 0=Failed 1=Sent 2=Pending
    size_t recv_status; // 1=Received 2=Read
} flat_message_store;


void ds_view_all_entries(flat_message_store fms[MSG_STORE_SIZE]);
void ds_resize_store();
void ds_add_message();
void ds_stream_messages_desc();
void ds_stream_user_messages_desc();
