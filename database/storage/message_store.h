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
    int    item_count;  // Increment with each new item
} flat_message_store;

void ms_view_all_entries(flat_message_store* fms);
void ms_resize_store();
void ms_add_message(char* sender_id, char* recipient_id, char* user_message,
                    flat_message_store* fms);
void ms_stream_messages_desc();
void ms_stream_user_messages_desc(char* sender_id);
