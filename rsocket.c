#include "rsocket.h"

recv_buf* recv_buf_table;
recv_msg_id* recv_msg_id_table;
unack_msg* unack_msg_table;

short msg_id = 0;
pthread_t X;
pthread_mutex_t lock;
int sockfd;
int recv_buf_start = 0, recv_buf_end = 0;
int unack_count = 0;
int char_count = 0, total_transmissions = 0;

int dropMessage(float p){
	float r = (float)rand() / (float)RAND_MAX;
	if(r < p)
		return 1;
	else
		return 0;
}

void HandleRetransmit(){
	time_t cur_time = time(NULL);
	char msg[MSG_MAX_SIZE], buf[MSG_MAX_SIZE+2];
	int msg_len,i,j,flag;
	struct sockaddr dest;
	short sent_msg_id;
	char buf2[10];

	
	for(i=0; i<TABLE_MAX_SIZE; i++){
		pthread_mutex_lock(&lock);
		if(unack_msg_table[i].id != -1){	// if ACK has not been received

			if(cur_time - unack_msg_table[i].sent_at >= T){	// if timeout period is over

				strcpy(msg, unack_msg_table[i].msg);
				msg_len = unack_msg_table[i].msg_len;
				dest = unack_msg_table[i].dest;
				sent_msg_id = unack_msg_table[i].id;
				flag = unack_msg_table[i].flag;

				sprintf(buf2, "%2hd", sent_msg_id);
				buf[0] = buf2[0];
				buf[1] = buf2[1];
				for(j=0; j<msg_len; j++){
					buf[j+2] = msg[j];
				}

				// retransmit the message and update the sent_at time
				sendto(sockfd, buf, msg_len+2, flag, (const struct sockaddr*)&dest, sizeof(dest));
				total_transmissions++;
				unack_msg_table[i].sent_at = cur_time;
			}
		}
		pthread_mutex_unlock(&lock);
	}
	
}

void HandleAppMsgRecv(char* buf, int n, struct sockaddr cliaddr){
	char buf2[10];
	memset(buf2, 0, sizeof(buf2));
	buf2[0]=buf[0];
	buf2[1]=buf[1];
	short recv_msg_id = atoi(buf2);

	sendto(sockfd, buf2, 2, 0, (struct sockaddr *)&cliaddr, sizeof(cliaddr)); //send ACK
	
	int i;

	if(recv_msg_id_table[recv_msg_id].id != -1){ //duplicate message
		return;
	}

	// not a duplicate, add to recv buffer
	for(i=0; i<n-2; i++){
		recv_buf_table[recv_buf_end].msg[i] = buf[i+2];
	}

	recv_buf_table[recv_buf_end].src = cliaddr;
	recv_buf_table[recv_buf_end].msg_len = n-2; 	//2 bytes for msg_id

	recv_buf_end++;

	// update recv_msg_id table
	recv_msg_id_table[recv_msg_id].id = recv_msg_id;

}

void HandleACKMsgRecv(char* buf){
	int i;
	char buf2[10];
	memset(buf2, 0, sizeof(buf2));
	buf2[0]=buf[0];
	buf2[1]=buf[1];
	short recv_msg_id = atoi(buf2);

	// ACK received, update unack_msg table and unack_count
	pthread_mutex_lock(&lock);
	unack_msg_table[recv_msg_id].id = -1;
	unack_count--;
	pthread_mutex_unlock(&lock);					
}

void HandleRecv(){
	int clilen;
	struct sockaddr cliaddr;
	clilen = sizeof(cliaddr);
	char buf[MSG_MAX_SIZE+2];
	memset(buf, 0, sizeof(buf));
	int n = recvfrom(sockfd, buf, MSG_MAX_SIZE+2, 0, &cliaddr, &clilen);

	int i;
	if(dropMessage(DROP_P) == 1)
		return;

	if(n > 2){	//AppMsg, msg_id and msg both are received
		HandleAppMsgRecv(buf,n, cliaddr);
	}
	else{					//ACK, only msg_id received
		HandleACKMsgRecv(buf);
	}
}

void* runner(void * param){
	fd_set readfds;
	int r;
	struct timeval t;
	t.tv_sec = T;
	t.tv_usec = 0;
	while(1){
		FD_ZERO(&readfds);
		FD_SET(sockfd, &readfds);
		r = select(sockfd+1, &readfds, 0, 0, &t);
		if(r < 0){
			continue;
		}
		if(FD_ISSET(sockfd, &readfds)){	//message received
			HandleRecv();
		}
		else{	//timeout
			HandleRetransmit();
			t.tv_sec = T;
			t.tv_usec = 0;
		}
	}
	pthread_exit(0);
}

int r_socket(int domain, int type, int protocol){
	if(type != SOCK_MRP)
		return -1;
	
	sockfd = socket(domain, SOCK_DGRAM, protocol);
	if(sockfd < 0){
		return -1;
	}


	// create thread X
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_create(&X,&attr,runner,NULL);

	pthread_mutex_init(&lock,NULL);
	srand(time(NULL));
	// dynamically allocate space for the tables and initialize them
	recv_buf_table = (recv_buf*)malloc(TABLE_MAX_SIZE * sizeof(recv_buf));
	recv_msg_id_table = (recv_msg_id*)malloc(TABLE_MAX_SIZE * sizeof(recv_msg_id));
	unack_msg_table = (unack_msg*)malloc(TABLE_MAX_SIZE * sizeof(unack_msg));

	int i;
	memset(unack_msg_table, 0, sizeof(unack_msg_table));
	memset(recv_msg_id_table, 0, sizeof(recv_msg_id_table));
	memset(recv_buf_table, 0, sizeof(recv_buf_table));

	for(i=0; i<TABLE_MAX_SIZE; i++){
		unack_msg_table[i].id = -1;
		recv_msg_id_table[i].id = -1;
	}

	return sockfd;
}

int r_bind(int sockfd, const struct sockaddr* addr, socklen_t addrlen){
	int r = bind(sockfd, addr, addrlen);
	return r; 
}	

ssize_t r_sendto(int sockfd, const void *buf, size_t len, int flags, const struct sockaddr *dest_addr, socklen_t addrlen){
	
	char* buf1 = (char*)buf;
	char buf2[MSG_MAX_SIZE+4];
	memset(buf2, 0, sizeof(buf2));

	char buf3[10];
	sprintf(buf3, "%2hd", msg_id);
	buf2[0] = buf3[0];
	buf2[1] = buf3[1];
	int i,r;
	for(i=0; i<len; i++){
		buf2[i+2] = buf1[i];
	}

	r = sendto(sockfd, buf2, len+2, flags, dest_addr, addrlen);
	char_count++;
	
	// add to unack table
	pthread_mutex_lock(&lock);
	total_transmissions++;
	
	unack_msg_table[msg_id].id = msg_id;
	strcpy(unack_msg_table[msg_id].msg, buf1);
	unack_msg_table[msg_id].dest = *dest_addr;
	unack_msg_table[msg_id].msg_len = r-2;
	unack_msg_table[msg_id].flag = flags;
	unack_msg_table[msg_id].sent_at = time(NULL);
	
	unack_count++;
	pthread_mutex_unlock(&lock);

	msg_id++;

	return r-2;	//2 bytes for msg_id
}

ssize_t r_recvfrom(int sockfd, void *buf, size_t len, int flag, struct sockaddr *src_addr, socklen_t *addrlen){

	while(recv_buf_start == recv_buf_end){ //recv buffer is empty
		sleep(1);
	}

	//read first message from recv buffer
	strcpy(buf, recv_buf_table[recv_buf_start].msg);
	*src_addr = recv_buf_table[recv_buf_start].src;
	*addrlen = sizeof(*src_addr);
	int msg_len = recv_buf_table[recv_buf_start].msg_len;

	recv_buf_start++;

	return msg_len;
}

int r_close(int sockfd){
	int i;
	//wait if ACK for some messages have not been received
	while(1){
		sleep(1);
		pthread_mutex_lock(&lock);
		if(unack_count == 0)
			break;
		pthread_mutex_unlock(&lock);
	}
	

	close(sockfd);
	pthread_cancel(X);
	free(recv_buf_table);
	free(recv_msg_id_table);
	free(unack_msg_table);
	pthread_mutex_destroy(&lock);
	printf("Average number of transmissions = %f\n",(float)total_transmissions/char_count);

}