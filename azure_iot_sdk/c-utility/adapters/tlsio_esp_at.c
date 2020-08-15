// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include "azure_c_shared_utility/tlsio.h"
#include "azure_c_shared_utility/tlsio_esp_at.h"
#include "azure_c_shared_utility/singlylinkedlist.h"
#include "azure_c_shared_utility/esp_at_socket.h"
#include "azure_c_shared_utility/optimize_size.h"
#include "azure_c_shared_utility/xlogging.h"

#define ESP_AT_XIO_RECEIVE_BUFFER_SIZE    128

typedef enum IO_STATE_TAG
{
    IO_STATE_CLOSED,
    IO_STATE_OPENING,
    IO_STATE_OPEN,
    IO_STATE_CLOSING,
    IO_STATE_ERROR
} IO_STATE;

typedef struct PENDING_TLS_IO_TAG
{
    unsigned char* bytes;
    size_t size;
    ON_SEND_COMPLETE on_send_complete;
    void* callback_context;
    SINGLYLINKEDLIST_HANDLE pending_io_list;
} PENDING_TLS_IO;

typedef struct TLS_IO_INSTANCE_TAG
{
    ESP_AT_SOCKET_HANDLE tcp_socket_connection;
    ON_BYTES_RECEIVED on_bytes_received;
    ON_IO_ERROR on_io_error;
    void* on_bytes_received_context;
    void* on_io_error_context;
    char* hostname;
    int port;
    IO_STATE io_state;
    SINGLYLINKEDLIST_HANDLE pending_io_list;
} TLS_IO_INSTANCE;

/*this function will clone an option given by name and value*/
static void* tlsio_esp_at_CloneOption(const char* name, const void* value)
{
    (void)name;
    (void)value;
    return NULL;
}

/*this function destroys an option previously created*/
static void tlsio_esp_at_DestroyOption(const char* name, const void* value)
{
    (void)name;
    (void)value;
}

static OPTIONHANDLER_HANDLE tlsio_esp_at_retrieveoptions(CONCRETE_IO_HANDLE tlsio_esp_at)
{
    OPTIONHANDLER_HANDLE result;
    (void)tlsio_esp_at;
    result = OptionHandler_Create(tlsio_esp_at_CloneOption, tlsio_esp_at_DestroyOption, tlsio_esp_at_setoption);
    if (result == NULL)
    {
        /*return as is*/
    }
    else
    {
        /*insert here work to add the options to "result" handle*/
    }
    return result;
}

static const IO_INTERFACE_DESCRIPTION tlsio_esp_at_interface_description =
{
    tlsio_esp_at_retrieveoptions,
    tlsio_esp_at_create,
    tlsio_esp_at_destroy,
    tlsio_esp_at_open,
    tlsio_esp_at_close,
    tlsio_esp_at_send,
    tlsio_esp_at_dowork,
    tlsio_esp_at_setoption
};

static void indicate_error(TLS_IO_INSTANCE* tlsio_esp_at_instance)
{
    if ((tlsio_esp_at_instance->io_state == IO_STATE_CLOSED)
        || (tlsio_esp_at_instance->io_state == IO_STATE_ERROR))
    {
        return;
    }
    tlsio_esp_at_instance->io_state = IO_STATE_ERROR;
    if (tlsio_esp_at_instance->on_io_error != NULL)
    {
        tlsio_esp_at_instance->on_io_error(tlsio_esp_at_instance->on_io_error_context);
    }
}

static int add_pending_io(TLS_IO_INSTANCE* tlsio_esp_at_instance, const unsigned char* buffer, size_t size, ON_SEND_COMPLETE on_send_complete, void* callback_context)
{
    int result;
    PENDING_TLS_IO* pending_tlsio_esp_at = (PENDING_TLS_IO*)malloc(sizeof(PENDING_TLS_IO));
    if (pending_tlsio_esp_at == NULL)
    {
        LogError("Allocation Failure: Unable to allocate pending list.");
        result = MU_FAILURE;
    }
    else
    {
        pending_tlsio_esp_at->bytes = (unsigned char*)malloc(size);
        if (pending_tlsio_esp_at->bytes == NULL)
        {
            LogError("Allocation Failure: Unable to allocate pending list.");
            free(pending_tlsio_esp_at);
            result = MU_FAILURE;
        }
        else
        {
            pending_tlsio_esp_at->size = size;
            pending_tlsio_esp_at->on_send_complete = on_send_complete;
            pending_tlsio_esp_at->callback_context = callback_context;
            pending_tlsio_esp_at->pending_io_list = tlsio_esp_at_instance->pending_io_list;
            (void)memcpy(pending_tlsio_esp_at->bytes, buffer, size);
            if (singlylinkedlist_add(tlsio_esp_at_instance->pending_io_list, pending_tlsio_esp_at) == NULL)
            {
                LogError("Failure: Unable to add socket to pending list.");
                free(pending_tlsio_esp_at->bytes);
                free(pending_tlsio_esp_at);
                result = MU_FAILURE;
            }
            else
            {
                result = 0;
            }
        }
    }

    return result;
}

#define NSAPI_ERROR_WOULD_BLOCK   -3001

static int retrieve_data(TLS_IO_INSTANCE* tlsio_esp_at_instance)
{
    int received = 1;
    int total_received = 0;

    unsigned char* recv_bytes = malloc(ESP_AT_XIO_RECEIVE_BUFFER_SIZE);
    if (recv_bytes == NULL)
    {
        LogError("Socketio_Failure: NULL allocating input buffer.");
        indicate_error(tlsio_esp_at_instance);
        return -1;
    }
    
    while (received > 0)
    {
        received = esp_at_socket_receive(tlsio_esp_at_instance->tcp_socket_connection, (char*)recv_bytes, ESP_AT_XIO_RECEIVE_BUFFER_SIZE);
        if (received > 0)
        {
            total_received += received;
            if (tlsio_esp_at_instance->on_bytes_received != NULL)
            {
                /* explictly ignoring here the result of the callback */
                tlsio_esp_at_instance->on_bytes_received(tlsio_esp_at_instance->on_bytes_received_context, recv_bytes, received);
            }
        }
        else if (received < 0)
        {
            if(received != NSAPI_ERROR_WOULD_BLOCK)     // NSAPI_ERROR_WOULD_BLOCK is not a real error but pending.
            {
                indicate_error(tlsio_esp_at_instance);
                LogError("Socketio_Failure: underlying IO error %d.", received);
                free(recv_bytes);
                return -1;
            }
        }
    }
    free(recv_bytes);
    
    return total_received;
}

static int send_queued_data(TLS_IO_INSTANCE* tlsio_esp_at_instance)
{
    int errors = 0;
    int sent = 0;
    
    LIST_ITEM_HANDLE first_pending_io = singlylinkedlist_get_head_item(tlsio_esp_at_instance->pending_io_list);
    while (first_pending_io != NULL)
    {
        PENDING_TLS_IO* pending_tlsio_esp_at = (PENDING_TLS_IO*)singlylinkedlist_item_get_value(first_pending_io);
        if (pending_tlsio_esp_at == NULL)
        {
            indicate_error(tlsio_esp_at_instance);
            return -1;
        }

        int send_result = esp_at_socket_send(tlsio_esp_at_instance->tcp_socket_connection, (const char*)pending_tlsio_esp_at->bytes, pending_tlsio_esp_at->size);
        if (send_result != (int)pending_tlsio_esp_at->size)
        {
            if (send_result == 0)
            {
                // The underlying network layer may encounter hardware / environment issues, 
                // but the driver doesn't handle it properly. So here the send API always return 0, 
                // this causes the program running into dead loop if not check it here.
                if (errors++ >= 10)
                {
                    // Treat it as a network error after try 10 times.
                    LogError("Socketio_Failure: encountered unknow connection issue, the connection will be restarted.");
                    indicate_error(tlsio_esp_at_instance);
                    return -1;
                }
                ThreadAPI_Sleep(10);
            }
            else if (send_result < 0)
            {
                indicate_error(tlsio_esp_at_instance);
                return -1;
            }
            else
            {
                /* send something, wait for the rest */
                memmove(pending_tlsio_esp_at->bytes, pending_tlsio_esp_at->bytes + send_result, pending_tlsio_esp_at->size - send_result);
                sent += send_result;
            }
        }
        else
        {
            sent += send_result;
            if (pending_tlsio_esp_at->on_send_complete != NULL)
            {
                pending_tlsio_esp_at->on_send_complete(pending_tlsio_esp_at->callback_context, IO_SEND_OK);
            }

            free(pending_tlsio_esp_at->bytes);
            free(pending_tlsio_esp_at);
            if (singlylinkedlist_remove(tlsio_esp_at_instance->pending_io_list, first_pending_io) != 0)
            {
                indicate_error(tlsio_esp_at_instance);
                return -1;
            }
            errors = 0;
        }

        first_pending_io = singlylinkedlist_get_head_item(tlsio_esp_at_instance->pending_io_list);
    }

    return sent;
}

static void close_tcp_connection(TLS_IO_INSTANCE* tlsio_esp_at_instance)
{
    if (tlsio_esp_at_instance->io_state != IO_STATE_CLOSED)
    {
        if (tlsio_esp_at_instance->tcp_socket_connection != NULL)
        {
            esp_at_socket_close(tlsio_esp_at_instance->tcp_socket_connection);
            esp_at_socket_destroy(tlsio_esp_at_instance->tcp_socket_connection);
            tlsio_esp_at_instance->tcp_socket_connection = NULL;
        }
        tlsio_esp_at_instance->io_state = IO_STATE_CLOSED;
    }
}

CONCRETE_IO_HANDLE tlsio_esp_at_create(void* io_create_parameters)
{
    TLSIO_CONFIG* tlsio_esp_at_config = io_create_parameters;
    TLS_IO_INSTANCE* result;

    if (tlsio_esp_at_config == NULL)
    {
        result = NULL;
    }
    else
    {    
        result = (TLS_IO_INSTANCE*)malloc(sizeof(TLS_IO_INSTANCE));
        if (result == NULL) {
            LogError("Socketio_Failure: NULL allocating socket io instance.");
        }
        else
        {
            result->pending_io_list = singlylinkedlist_create();
            if (result->pending_io_list == NULL)
            {
                free(result);
                result = NULL;
            }
            else
            {
                result->hostname = strdup(tlsio_esp_at_config->hostname);
                if (result->hostname == NULL)
                {
                    singlylinkedlist_destroy(result->pending_io_list);
                    free(result);
                    result = NULL;
                }
                else
                {
                    result->port = tlsio_esp_at_config->port;
                    result->on_bytes_received = NULL;
                    result->on_io_error = NULL;
                    result->on_bytes_received_context = NULL;
                    result->on_io_error_context = NULL;
                    result->io_state = IO_STATE_CLOSED;
                    result->tcp_socket_connection = NULL;
                }
            }
        }
    }

    return result;
}

void tlsio_esp_at_destroy(CONCRETE_IO_HANDLE tlsio_esp_at)
{
    if (tlsio_esp_at != NULL)
    {
        TLS_IO_INSTANCE* tlsio_esp_at_instance = (TLS_IO_INSTANCE*)tlsio_esp_at;

        // Close the tcp connection
        close_tcp_connection(tlsio_esp_at_instance);
    
        // Clear all pending IOs
        LIST_ITEM_HANDLE first_pending_io = singlylinkedlist_get_head_item(tlsio_esp_at_instance->pending_io_list);
        while (first_pending_io != NULL)
        {
            PENDING_TLS_IO* pending_tlsio_esp_at = (PENDING_TLS_IO*)singlylinkedlist_item_get_value(first_pending_io);
            if (pending_tlsio_esp_at != NULL)
            {
                free(pending_tlsio_esp_at->bytes);
                free(pending_tlsio_esp_at);
            }

            (void)singlylinkedlist_remove(tlsio_esp_at_instance->pending_io_list, first_pending_io);
            first_pending_io = singlylinkedlist_get_head_item(tlsio_esp_at_instance->pending_io_list);
        }
        singlylinkedlist_destroy(tlsio_esp_at_instance->pending_io_list);
    
        if(tlsio_esp_at_instance->hostname != NULL)
        {
            free(tlsio_esp_at_instance->hostname);
            tlsio_esp_at_instance->hostname = NULL;
        }
        free(tlsio_esp_at);
    }
}

int tlsio_esp_at_open(CONCRETE_IO_HANDLE tlsio_esp_at, ON_IO_OPEN_COMPLETE on_io_open_complete, void* on_io_open_complete_context, ON_BYTES_RECEIVED on_bytes_received, void* on_bytes_received_context, ON_IO_ERROR on_io_error, void* on_io_error_context)
{
    int result;
    TLS_IO_INSTANCE* tlsio_esp_at_instance = (TLS_IO_INSTANCE*)tlsio_esp_at;
    

    if (tlsio_esp_at_instance == NULL ||
        tlsio_esp_at_instance->io_state != IO_STATE_CLOSED)
    {
        result = MU_FAILURE;
    }
    else
    {
        tlsio_esp_at_instance->tcp_socket_connection = esp_at_socket_create(true);
        if (tlsio_esp_at_instance->tcp_socket_connection == NULL)
        {
            result = MU_FAILURE;
        }
        else
        {
            if (esp_at_socket_connect(tlsio_esp_at_instance->tcp_socket_connection, tlsio_esp_at_instance->hostname, tlsio_esp_at_instance->port) != 0)
            {
                esp_at_socket_destroy(tlsio_esp_at_instance->tcp_socket_connection);
                tlsio_esp_at_instance->tcp_socket_connection = NULL;
                result = MU_FAILURE;
            }
            else
            {
                esp_at_socket_set_blocking(tlsio_esp_at_instance->tcp_socket_connection, false, 0);

                tlsio_esp_at_instance->on_bytes_received = on_bytes_received;
                tlsio_esp_at_instance->on_bytes_received_context = on_bytes_received_context;

                tlsio_esp_at_instance->on_io_error = on_io_error;
                tlsio_esp_at_instance->on_io_error_context = on_io_error_context;

                tlsio_esp_at_instance->io_state = IO_STATE_OPEN;

                result = 0;
            }
        }
    }
    
    if (on_io_open_complete != NULL)
    {
        on_io_open_complete(on_io_open_complete_context, result == 0 ? IO_OPEN_OK : IO_OPEN_ERROR);
    }

    return result;
}

int tlsio_esp_at_close(CONCRETE_IO_HANDLE tlsio_esp_at, ON_IO_CLOSE_COMPLETE on_io_close_complete, void* callback_context)
{
    int result = 0;

    if (tlsio_esp_at == NULL)
    {
        result = MU_FAILURE;
    }
    else
    {
        TLS_IO_INSTANCE* tlsio_esp_at_instance = (TLS_IO_INSTANCE*)tlsio_esp_at;
        if (tlsio_esp_at_instance->io_state == IO_STATE_CLOSED || 
            tlsio_esp_at_instance->io_state == IO_STATE_ERROR)
        {
            result = MU_FAILURE;
        }
        else
        {
            close_tcp_connection(tlsio_esp_at_instance);
            if (on_io_close_complete != NULL)
            {
                on_io_close_complete(callback_context);
            }
            result = 0;
        }
    }
    
    return result;
}

int tlsio_esp_at_send(CONCRETE_IO_HANDLE tlsio_esp_at, const void* buffer, size_t size, ON_SEND_COMPLETE on_send_complete, void* callback_context)
{
    int result;

    if ((tlsio_esp_at == NULL) ||
        (buffer == NULL) ||
        (size == 0))
    {
        /* Invalid arguments */
        result = MU_FAILURE;
    }
    else
    {
        result = 0;

        TLS_IO_INSTANCE* tlsio_esp_at_instance = (TLS_IO_INSTANCE*)tlsio_esp_at;
        if (tlsio_esp_at_instance->io_state != IO_STATE_OPEN)
        {
            result = MU_FAILURE;
        }
        else
        {
            // Queue the data, and the tlsio_esp_at_dowork sends the package later
            if (add_pending_io(tlsio_esp_at_instance, buffer, size, on_send_complete, callback_context) != 0)
            {
                result = MU_FAILURE;
            }
        }
    }

    return result;
}

void tlsio_esp_at_dowork(CONCRETE_IO_HANDLE tlsio_esp_at)
{
    if (tlsio_esp_at != NULL)
    {
        TLS_IO_INSTANCE* tlsio_esp_at_instance = (TLS_IO_INSTANCE*)tlsio_esp_at;
        if (tlsio_esp_at_instance->io_state == IO_STATE_OPEN)
        {
            // Retrieve all data from IoT Hub
            if (retrieve_data(tlsio_esp_at_instance) < 0)
            {
                return;
            }
        
            // Send all packages in the queue
            send_queued_data(tlsio_esp_at_instance);
        }
    }
}

int tlsio_esp_at_setoption(CONCRETE_IO_HANDLE tlsio_esp_at, const char* optionName, const void* value)
{
    /* Not implementing any options, do nothing */
    return OPTIONHANDLER_OK;
}

const IO_INTERFACE_DESCRIPTION* tlsio_esp_at_get_interface_description(void)
{
    return &tlsio_esp_at_interface_description;
}
