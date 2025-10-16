#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "message_store.h"

int* end_of_db_ptr = NULL;

void ms_view_all_entries(flat_message_store* fms)
{

    printf("Message Store sender_id: %s\n", fms->sender_id);
    printf("Message Store recipient_id: %s\n", fms->recipient_id);
    printf("Message Store msg_len: %zu\n", fms->msg_len);
    printf("Message Store message: %s\n", fms->message);
    printf("Message Store send_time: %jd\n", fms->send_time);
    printf("Message Store recv_time: %jd\n", fms->recv_time);
    printf("Message Store msg_type: %c\n", fms->msg_type);
    // TODO: Fix the char pointer print above
    printf("Message Store send_status: %zu\n", fms->send_status);
    printf("Message Store recv_status: %zu\n", fms->recv_status);

    int i = 1;
    printf("Starting at item: %d\n", fms[i - 1].item_count);
    while (fms[i].item_count > fms[i - 1].item_count)
    {
        printf("\nIteration num: %d\n", i);
        printf("Message Store sender_id: %s\n", fms[i].sender_id);
        printf("Message Store recipient_id: %s\n", fms[i].recipient_id);
        printf("Message Store msg_len: %zu\n", fms[i].msg_len);
        printf("Message Store message: %s\n", fms[i].message);
        printf("Message Store send_time: %jd\n", fms[i].send_time);
        printf("Message Store recv_time: %jd\n", fms[i].recv_time);
        printf("Message Store msg_type: %c\n", fms[i].msg_type);
        // TODO: Fix the char pointer print above
        printf("Message Store send_status: %zu\n", fms[i].send_status);
        printf("Message Store recv_status: %zu\n", fms[i].recv_status);
        i++;
    }
}

int* ms_point_to_last_entry(flat_message_store* fms)
{
    int i = 1;
    while (fms[i].item_count > fms[i - 1].item_count)
    {
        i++;
    }
    printf("Number of items saved: %d\n", fms[i - 1].item_count);
    return &fms[i - 1].item_count;
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
                    time_t* sent_time, time_t* recieved_time,
                    flat_message_store* fms, int** end_of_db_ptr)
{
    int index = **end_of_db_ptr;
    strcpy(fms[index].sender_id, sender_id);
    strcpy(fms[index].recipient_id, recipient_id);
    fms[index].msg_len = strlen(user_message);
    fms[index].message = malloc(fms[index].msg_len + 1);
    strcpy(fms[index].message, user_message);
    fms[index].send_time   = time(sent_time);
    fms[index].recv_time   = time(recieved_time);
    fms[index].send_status = 1;
    fms[index].recv_status = 1;
    fms[index].item_count  = index;
    *end_of_db_ptr         = &fms[index].item_count;
    return;

    // IMPROVEMENT:
    // - Find out how to not rely on having a list end flag
    // - This function needs to work in conflicting cases
}

// TODO: Implement resizing mechanism when the store fills up
void ms_resize_store() { return; }

void free_memory(flat_message_store* fms)
{
    printf("Freeing memory allocated to messages\n");
    int i = 1;
    free(fms[0].message);
    while (fms[i].item_count > fms[i - 1].item_count)
    {
        free(fms[i].message);
        i++;
    }
    return;
}

int main()
{
    flat_message_store fms[MSG_STORE_SIZE];
    time_t             now = time(NULL);
    printf("Message store initalized\n\n");

    // TEST: Adding this as an examples message
    strcpy(fms[0].sender_id, "christian");
    strcpy(fms[0].recipient_id, "alejandro");
    fms[0].msg_len = 40;
    fms[0].message = malloc(fms[0].msg_len);
    strcpy(fms[0].message, "Msg % from *7323 User");
    fms[0].send_time   = time(NULL);
    fms[0].recv_time   = time(NULL);
    fms[0].send_status = 1;
    fms[0].recv_status = 1;
    fms[0].item_count  = 1;
    // TEST: Adding this as an examples message

    // ms_add_message("chris", "nj", "test-message", &now, &now, fms,
    //                &end_of_db_ptr);
    ms_view_all_entries(fms);
    int* end_of_db_ptr = ms_point_to_last_entry(fms);

    // TODO: Free all memory
    free_memory(fms);

    return (0);
}
