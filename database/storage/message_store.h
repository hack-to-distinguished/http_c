#include <time.h>

#define USER_ID_SIZE 256
#define MSG_STORE_SIZE 1000

typedef struct
{
    int    ID; // Increment with each new item
    char   sender_id[USER_ID_SIZE];
    char   recipient_id[USER_ID_SIZE];
    char*  message;
    size_t msg_len;
    time_t send_time; // Likely only storing time here
    time_t recv_time; // Likely only storing time here
    char   msg_type;
    size_t send_status; // 0=Failed 1=Sent 2=Pending
    size_t recv_status; // 1=Received 2=Read
} flat_message_store;

extern int* end_of_db_ptr;

int* ms_point_to_last_entry(flat_message_store* fms);
void ms_view_all_entries(flat_message_store* fms);
void ms_resize_store();
void ms_add_message(char* sender_id, char* recipient_id, char* user_message,
                    time_t* sent_time, time_t* recieved_time,
                    flat_message_store* fms, int** end_of_db_ptr);
void ms_stream_messages_desc(flat_message_store* fms, int** end_of_db_ptr);
void ms_stream_user_messages_desc(flat_message_store* fms, int** end_of_db_ptr,
                                  char* sender_id);
void ms_show_latest_msg();
void free_memory(flat_message_store* fms);
