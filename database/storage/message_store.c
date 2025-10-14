#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "message_store.h"

void ds_view_all_entries(flat_message_store fms[MSG_STORE_SIZE])
{

    int msg_num = MSG_STORE_SIZE;
    printf("Number of messages saved: %d\n", msg_num);
    for (int i = 0; i < msg_num; i++)
    {
        printf("Message Store sender_id: %s\n", fms[i].sender_id);
        printf("Message Store recipient_id: %s\n", fms[i].recipient_id);
        printf("Message Store msg_len: %zu\n", fms[i].msg_len);
        printf("Message Store send_time: %jd\n", fms[i].send_time);
        printf("Message Store recv_time: %jd\n", fms[i].recv_time);
        printf("Message Store msg_type: %c\n", fms[i].msg_type);
        // TODO: Fix the char pointer print above
        printf("Message Store send_status: %zu\n", fms[i].send_status);
        printf("Message Store recv_status: %zu\n", fms[i].recv_status);
        if (fms[i].last_item == 1)
        {
            break;
        }
    }
}

// TODO: Implement resizing mechanism when the store fills up
void ds_resize_store() { return; }

// TODO: implement the data stucture in the existing messaging system,
// appending information to message store as they come in
// - Add entry
// - Get latest entry
// - Get entry in reverse order
void ds_add_message() { return; }
void ds_stream_messages_desc() { return; }
void ds_stream_user_messages_desc() { return; }

// TODO: Add cleanup pointers

int main()
{
    printf("Message store initalized\n\n");
    time_t *sent_time, *recieved_time;

    // TEST: Adding this as an examples message
    flat_message_store fms[MSG_STORE_SIZE];
    strcpy(fms[0].sender_id, "christian");
    strcpy(fms[0].recipient_id, "alejandro");
    fms[0].msg_len     = 20;
    fms[0].send_time   = time(sent_time);
    fms[0].recv_time   = time(recieved_time);
    fms[0].send_status = 1;
    fms[0].recv_status = 1;
    fms[0].last_item   = 1;
    // TEST: Adding this as an examples message

    ds_view_all_entries(fms);

    return (0);
}
