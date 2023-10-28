# Reliable-Communication-Library

This library provides reliable communication over an unreliable link. It provides a message-oriented, reliable, exactly-once delivery communication layer over UDP sockets using timeout and retransmissions.

The file rsocket.h contains #includes for the sockets to work, #define for timeout T and for drop probability DROP_P. It contains the function prototypes for these functions - r_socket, r_bind, r_sendto, r_recvfrom and r_close.
It also contains the struct definitions for the receive buffer, unacknowledged-message table and received-message-id table. The file rsocket.c contains the definitions of those functions and also the functions for handling receive and handling retransmission.


The messages used and their formats are as follows - 

1) Application message - This message is of more than two bytes. The first two bytes contain the message id and the bytes after that contain the actual message.

2) ACK message - This message is of exactly two bytes and contains the message id of the message that is acknowledged.


The data structures used are - 

1) recv_buf_table - this is the receive buffer which is an array of struct recv_buf.

	recv_buf has the following fields - 
		char msg[] - actual message
		int msg_len - length of the actual message
		struct sockaddr src - source address

	recv_buf_table uses two pointers recv_buf_start and recv_buf_end to denote which entries in recv_buf_table are valid.

2) recv_msg_id_table - this is an array of recv_msg_id and is implemented as a hash table where message id is the index in the array. 
	recv_msg_id contains - 
			short id - message id

3) unack_msg_table - this is an array of unack_msg and is implemented as a hash table where message id is the index in the array.
	unack_msg contains - 
		short id - message id
		struct sockaddr dest - destination address
		char msg[] - actual message
		int msg_len - length of the message
		int flag - flag used in r_sendto call
		time_t sent_at - time at which message was sent
