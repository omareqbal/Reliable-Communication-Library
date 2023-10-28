#include "rsocket.h"

int main() { 
    int sockfd; 
    struct sockaddr_in user1_addr, user2_addr; 
  
    // Creating socket file descriptor 
    sockfd = r_socket(AF_INET, SOCK_MRP, 0);
    
    if (sockfd < 0) { 
        perror("socket creation failed"); 
        exit(EXIT_FAILURE); 
    } 
  
    memset(&user1_addr, 0, sizeof(user1_addr));
    memset(&user2_addr, 0, sizeof(user2_addr)); 
      
    user1_addr.sin_family = AF_INET; 
    user1_addr.sin_addr.s_addr = INADDR_ANY; 
    user1_addr.sin_port = htons(50000+2*3043);

    user2_addr.sin_family = AF_INET; 
    user2_addr.sin_addr.s_addr = INADDR_ANY; 
    user2_addr.sin_port = htons(50000+2*3043+1);     

    if(r_bind(sockfd, (const struct sockaddr *)&user2_addr, sizeof(user2_addr)) < 0){
        perror("bind failed"); 
        exit(EXIT_FAILURE); 
    }
      
    char buf;
    int i, len;
    while(1){
        len = sizeof(user1_addr);
        r_recvfrom(sockfd, (char *)&buf, 1, 0, (struct sockaddr*) &user1_addr, &len);
        printf("%c",buf);
        fflush(stdout);
    }
    r_close(sockfd); 
    return 0; 
}