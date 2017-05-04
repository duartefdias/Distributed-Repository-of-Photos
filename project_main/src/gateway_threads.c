#include "../include/gateway.h"

pthread_mutex_t client_list_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t peer_list_mutex = PTHREAD_MUTEX_INITIALIZER;

// TODO
/* THREAD: receive peer address and add it to Peers linked list */
void* peerRecvThread(void* args)
{

}

//Thread always checking for new peer connections (INFINITE LOOP)
void* masterPeerRecvThread(void* args)
{
    int ret_val_recv = 0;
    int ret_val_pthread_create = 0;
    pthread_t* clients_id = NULL;
    pthread_t* peers_id = NULL;
    unsigned int i=0, j=0;
    Message_gw message_gw;

        clients_id = (pthread_t*)malloc(HUGE_NUMBER * sizeof(pthread_t));
        peers_id = (pthread_t*)malloc(HUGE_NUMBER * sizeof(pthread_t));

        while(true){
            //memset((void*)&client_socket_address, 0, sizeof(client_socket_address));

            fprintf(stderr, "recdd\n");

            if(message_gw.type == CLIENT_ADDRESS){
                ret_val_pthread_create = pthread_create( &clients_id[i], NULL, &clientthread, (void *)&client_thread_args);
                if (ret_val_pthread_create != 0) {
                    fprintf(stderr, "recv_pthread_create error!\n");
                    exit(EXIT_FAILURE);
                }
                i++;
            }else if(message_gw.type == PEER_ADDRESS){
                ret_val_pthread_create = pthread_create( &peers_id[j], NULL, &clientthread, (void *)&client_thread_args);
                if(ret_val_pthread_create != 0){
                    fprintf(stderr, "recv_pthread_create error!\n");
                    exit(EXIT_FAILURE);
                }
                j++;
            }
        }

    free(clients_id);
    free(peers_id);
}

// TODO
/* THREAD: receive client address and add it to Clients linked list */
void* clientRecvThread(void* args)
{
    int socket_fd = 0;
    int ret_val_send_to = 0;
    ClientRecvThreadArgs* client_recv_thread_args = NULL;
    SinglyLinkedList* peer_list_head = NULL;
    SinglyLinkedList* aux_peer_list_node = NULL;
    SinglyLinkedList* client_list_head = NULL;
    SinglyLinkedList* aux_client_list_node = NULL;
    SinglyLinkedList* new_client_list_node = NULL;
    struct sockaddr_in client_socket_address;
    struct sockaddr_in peer_socket_address;
    ClientProperties* clientProperties = NULL;
    Message_gw message_gw;

        message_gw.type = PEER_UNAVAILABLE;
        client_recv_thread_args = (ClientRecvThreadArgs*)args;
        socket_fd = client_recv_thread_args->socket_fd;
        peer_list_head = client_recv_thread_args->peer_list_head;
        client_list_head = client_recv_thread_args->client_list_head;
        client_socket_address = client_recv_thread_args->client_address;

        // create client list item payload
        clientProperties = (ClientProperties*)malloc(sizeof(ClientProperties));
        memset(&(clientProperties->client_socket_address), 0, sizeof(struct sockaddr_in));
        memset(&(clientProperties->connected_peer_socket_address), 0, sizeof(struct sockaddr_in));
        clientProperties -> client_socket_address = client_socket_address;

        // CRITICAL SECTION START
        pthread_mutex_lock(&client_list_mutex);


        // TODO: before search for the client in the list, search the list to see if it is already there
        for(aux_client_list_node = client_list_head; SinglyLinkedList_getNextNode(aux_client_list_node) != NULL; aux_client_list_node = SinglyLinkedList_getNextNode(aux_client_list_node)){
            
        }


        for(aux_client_list_node = client_list_head; SinglyLinkedList_getNextNode(aux_client_list_node) != NULL; aux_client_list_node = SinglyLinkedList_getNextNode(aux_client_list_node)){} // traverse list to last node
        if(SinglyLinkedList_getItem(aux_client_list_node) == NULL){ // If we are at the head, the node will already exist but item is null
            SinglyLinkedList_setItem(aux_client_list_node, clientProperties);
        }else{ // otherwise, alloc new node, set item and insert it at the end of the list
            new_client_list_node = SinglyLinkedList_newNode(NULL);
            SinglyLinkedList_setItem(new_client_list_node, clientProperties);
            SinglyLinkedList_insertAtEnd(aux_client_list_node, new_client_list_node);
            aux_client_list_node = new_client_list_node;
        }
        // at this aux_client_list_node points to the node with the new client
        pthread_mutex_unlock(&client_list_mutex);
        // CRITICAL SECTION END

        // search peer list for available peer to send to client
        // CRITICAL SECTION START
        pthread_mutex_lock(&peer_list_mutex);
        for(aux_peer_list_node = peer_list_head; SinglyLinkedList_getNextNode(aux_peer_list_node) != NULL; aux_peer_list_node = SinglyLinkedList_getNextNode(aux_peer_list_node)){
            if(((PeerProperties*)SinglyLinkedList_getItem(aux_peer_list_node))->status == PEER_AVAILABLE){
                message_gw.type = PEER_ADDRESS;
                peer_socket_address = ((PeerProperties*)SinglyLinkedList_getItem(aux_peer_list_node))->peer_socket_address;
                pthread_mutex_lock(&client_list_mutex);
                ((ClientProperties*)SinglyLinkedList_getItem(aux_client_list_node))->connected_peer_socket_address = peer_socket_address;
                pthread_mutex_lock(&client_list_mutex);
                message_gw.address = peer_socket_address.sin_addr.s_addr;
                message_gw.port = peer_socket_address.sin_port;
                ((PeerProperties*)SinglyLinkedList_getItem(aux_peer_list_node))->status = PEER_UNAVAILABLE;
                break;
            }
        }
        pthread_mutex_unlock(&peer_list_mutex);
        // CRITTICAL SECTION END

        // send message_gw to client
        ret_val_send_to = sendto(socket_fd, &message_gw, sizeof(message_gw), NO_FLAGS, (struct sockaddr *)&client_socket_address, sizeof(client_socket_address));

        // We free the args which were allocated in the calling thread; must free list of clients in main
        free(client_recv_thread_args);

    pthread_exit(0);
}

//Thread always checking for new client connections (INFINITE LOOP)
void* masterClientRecvThread(void* args)
{
    long int i = 0;
    int socket_fd = 0;
    int ret_val_recvfrom = 0;
    int ret_val_pthread_create = 0;
    pthread_t* thread_client_recv_id = NULL;
    struct sockaddr_in client_socket_address;
    socklen_t client_socket_address_len = sizeof(client_socket_address);
    Message_gw message_gw;
    MasterClientRecvThreadArgs* masterClientRecvThreadArgs = NULL;
    ClientRecvThreadArgs* clientRecvThreadArgs = NULL;

        // TODO: big enough? when to increase? how to increase?
        thread_client_recv_id = (pthread_t*)malloc(HUGE_NUMBER * sizeof(pthread_t));
        masterClientRecvThreadArgs = (MasterClientRecvThreadArgs*)args;
        socket_fd = masterClientRecvThreadArgs->socket_fd;

        while(true){
            memset((void*)&client_socket_address, 0, sizeof(client_socket_address));
            ret_val_recvfrom = recvfrom(socket_fd, &message_gw, sizeof(Message_gw), NO_FLAGS, (struct sockaddr *)&client_socket_address, &client_socket_address_len);

            if(message_gw.type == CLIENT_ADDRESS){
                clientRecvThreadArgs = (ClientRecvThreadArgs*)malloc(sizeof(ClientRecvThreadArgs));
                clientRecvThreadArgs->socket_fd = socket_fd;
                clientRecvThreadArgs->peer_list_head = masterClientRecvThreadArgs->peer_list_head;
                clientRecvThreadArgs->client_list_head = masterClientRecvThreadArgs->client_list_head;
                clientRecvThreadArgs->client_address = client_socket_address;
                ret_val_pthread_create = pthread_create(&thread_client_recv_id[i], NULL, &clientRecvThread, (void *)clientRecvThreadArgs);
                if(ret_val_pthread_create != 0){
                    fprintf(stderr, "recv_pthread_create (client - slave) error!\n");
                    exit(EXIT_FAILURE);
                }
                i += 1;
            }else{
                fprintf(stdout, "Client receive thread error: bad message type\n");
            }
        }

    free(thread_client_recv_id);
}

//Thread that sends peer address to new client
void* send_address_to_client(void* args)
{

}

//Thread that receives address from new peer
void* receive_address_from_peer(void * args)
{

}
