#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "message_store.h"

void ms_view_all_entries(flat_message_store* fms)
{
    // INFO: Start at 1 bc the first msg is empty
    int i = 1;
    while (fms[i].ID > fms[i - 1].ID)
    {
        printf("\nIteration num: %d\n", i);
        printf("Message Store ID: %d\n", fms[i].ID);
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
    printf("Messages printed: %d\n\n", i);
}

int* ms_point_to_last_entry(flat_message_store* fms)
{
    int i = 1;
    while (fms[i].ID > fms[i - 1].ID)
    {
        i++;
    }
    printf("End of list at: %d\n", fms[i - 1].ID);
    return &fms[i - 1].ID;
}

// TODO: Create get latest entry
void ms_stream_messages_desc(flat_message_store* fms, int** end_of_db_ptr)
{
    // TODO:
    // - Read the saved data
    // - Send data out in desc order:
    //    - get the end_of_db_ptr, while loop backwards
    // - Stream the results as they become available

    printf("\nStreaming messages\n");
    int index = **end_of_db_ptr;
    while (fms[index - 1].ID < fms[index].ID)
    {
        // TODO: Change this to a stream to page or something
        printf("%s\n", fms[index].message);
        index--;
    }
    return;
}

void ms_show_latest_msg();

void ms_stream_user_messages_desc(flat_message_store* fms, int** end_of_db_ptr,
                                  char* sender_id)
{
    // INFO: Creating linked lists between a users message would make
    // getting those user's message much faster
    // TODO:
    // - Read the saved data
    // - Filter by sender_id (How does SQL have such fast filters)
    // - Stream the results as they become available

    printf("\nGetting %s's messages\n", sender_id);
    int index = **end_of_db_ptr;
    while (fms[index - 1].ID < fms[index].ID)
    {
        // TODO: Change this to a stream to page or something
        if (strcmp(fms[index].sender_id, sender_id))
        {
            printf("%s\n", fms[index].message);
        }
        index--;
    }
    return;
    return;
}

void ms_add_message(char* sender_id, char* recipient_id, char* user_message,
                    time_t* sent_time, time_t* recieved_time,
                    flat_message_store* fms, int** end_of_db_ptr)
{
    int index = **end_of_db_ptr;
    index++;
    printf("Inserting data at index %d\n", index);

    strcpy(fms[index].sender_id, sender_id);
    strcpy(fms[index].recipient_id, recipient_id);
    fms[index].msg_len = strlen(user_message);
    fms[index].message = malloc(fms[index].msg_len + 1);
    strcpy(fms[index].message, user_message);
    fms[index].send_time   = time(sent_time);
    fms[index].recv_time   = time(recieved_time);
    fms[index].send_status = 1;
    fms[index].recv_status = 1;
    fms[index].ID          = index;

    *end_of_db_ptr = &fms[index].ID;
    return;

    // IMPROVEMENT:
    // - Find out how to not rely on having a list end flag
    // - This function needs to work in conflicting cases
}

void ms_resize_store()
{
    // TODO: Implement resizing mechanism when the store fills up
    return;
}

void free_memory(flat_message_store* fms)
{
    printf("Freeing memory allocated to messages\n");
    int i = 1;
    free(fms[0].message);
    while (fms[i].ID > fms[i - 1].ID)
    {
        free(fms[i].message);
        i++;
    }
    return;
}

int main()
{
    // INFO: The blow is for testing purposes
    flat_message_store fms[MSG_STORE_SIZE];
    printf("Message store initalized\n\n");
    int*   end_of_db_ptr = &fms[0].ID;
    time_t now           = time(NULL);

    ms_add_message("Christian", "Juan", "Another One", &now, &now, fms,
                   &end_of_db_ptr);

    ms_add_message("chris", "nj", "test-message", &now, &now, fms,
                   &end_of_db_ptr);
    ms_add_message("Alejandro", "Christian", "test message 2", &now, &now, fms,
                   &end_of_db_ptr);
    // ms_view_all_entries(fms);

    ms_stream_messages_desc(fms, &end_of_db_ptr);
    ms_stream_user_messages_desc(fms, &end_of_db_ptr, "Christian");

    // end_of_db_ptr = ms_point_to_last_entry(fms);

    free_memory(fms);
    return (0);
}
