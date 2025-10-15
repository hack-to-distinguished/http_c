#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "message_store.h"

void ms_view_all_entries(flat_message_store* fms)
{

    printf("Message Store sender_id: %s\n", fms->sender_id);
    printf("Message Store recipient_id: %s\n", fms->recipient_id);
    printf("Message Store msg_len: %zu\n", fms->msg_len);
    printf("Message Store send_time: %jd\n", fms->send_time);
    printf("Message Store recv_time: %jd\n", fms->recv_time);
    printf("Message Store msg_type: %c\n", fms->msg_type);
    // TODO: Fix the char pointer print above
    printf("Message Store send_status: %zu\n", fms->send_status);
    printf("Message Store recv_status: %zu\n", fms->recv_status);

    int i = 1;
    while (fms[i].item_count > fms[i - 1].item_count)
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
    }
}

// TODO: Create get latest entry
void ms_stream_messages_desc()
{
    // TODO:
    // - Read the saved data
    // - Stream the results as they become available
    return;
}
void ms_stream_user_messages_desc(char* sender_id)
{
    // TODO:
    // - Read the saved data
    // - Filter by sender_id (How does SQL have such fast filters)
    // - Stream the results as they become available
    return;
}

// TODO: Implement a add message function
void ms_add_message(char* sender_id, char* recipient_id, char* user_message,
                    flat_message_store* fms)
{
    // TODO:
    // - append the user message to the struct
    // - move the item_count flag

    fms->item_count = 0;
    return;

    // TODO: Should I create a linked list?
    // Would that make appending faster?

    // IMPROVEMENT:
    // - Find out how to not rely on having a list end flag
    // - This function needs to work in conflicting cases
}

// TODO: Implement resizing mechanism when the store fills up
void ms_resize_store() { return; }

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
    fms[0].item_count  = 1;
    // TEST: Adding this as an examples message

    ms_view_all_entries(fms);

    return (0);
}
