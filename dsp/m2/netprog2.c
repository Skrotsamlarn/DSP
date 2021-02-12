//Example code: A simple server side code, which echos back the received message. 
//Handle multiple socket connections with select and fd_set on Linux  
#include <stdio.h>  
#include <string.h>   //strlen  
#include <stdlib.h>   // strcmp
#include <errno.h>  
#include <unistd.h>   //close  
#include <arpa/inet.h>    //close  
#include <sys/types.h>  
#include <sys/socket.h>  
#include <netinet/in.h>  
#include <sys/time.h> //FD_SET, FD_ISSET, FD_ZERO macros  
#include <stdbool.h> // true, false

     
#define TRUE   1  
#define FALSE  0  
#define PORT 8888

volatile sig_atomic_t incoming_signal = false;

void signal_handler(int s)
{
    fputs("\nSending goodbye to clients\nShutting down, goodbye\n", stderr);
    incoming_signal = true;
}

int main(int argc , char *argv[]) { 
    int opt = TRUE;   
    int master_socket , addrlen , new_socket , client_socket[30] ,  
          max_clients = 30 , activity, i , valread , sd;   
    int max_sd;   
    struct sockaddr_in address;   // internet style
         
    char buffer[1025];  //data buffer of 1K  WHY?
         
    //set of socket descriptors
      
    fd_set readfds;
    char *message = "ECHO Daemon v1.0 \r\n";   
   
    signal(SIGINT, signal_handler);


    //initialise all client_socket[] to 0 so not checked  
    for (i = 0; i < max_clients; i++)   // nollställ våra sockets
    {   
        client_socket[i] = 0;   
    }   
        
    //create a master socket  
    // returns a file descriptor to master_socket
    if( (master_socket = socket(AF_INET , SOCK_STREAM , 0)) < 0)   // vi laddar in att det är internet, och sedan byte stream aka tcp, och 0 är att det är den första sortens SOCK_STREAM(av 1)
    {   
        perror("socket failed");   
        exit(EXIT_FAILURE);   
    }  
    
    //set master socket to allow multiple connections ,  
    //this is just a good habit, it will work without this  
    if( setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt,  
        sizeof(opt)) < 0 )   // tillåt plural anslutningar
    {   
        perror("setsockopt");   
        exit(EXIT_FAILURE);   
    }   
    
    //type of socket created  
    address.sin_family = AF_INET;         // ladda inte familj, vilket i detta fall är internet
    address.sin_addr.s_addr = INADDR_ANY; // ladda in IPadress
    address.sin_port = htons( PORT );     // ladda in port, detta behöver gå genom htons för att göras om från host byte order till network byte order
    
    //bind the socket to localhost port 8888  
    if (bind(master_socket, (struct sockaddr *)&address, sizeof(address))<0)   // gör om adressen till intern socket som kernel kan jobba med
    {                                                                          // vi kopplar ihop mastersocket till adressen. om den returnerar < 0 så blev det lite fel så vi returnerar error
        perror("bind failed");   
        exit(EXIT_FAILURE);   
    }   
    printf("Listener on port %d \n", PORT);   
        
    //try to specify maximum of 3 pending connections for the master socket  
    if (listen(master_socket, 3) < 0)   // kolla så vi inte har fler än 3 stycken samtidigt som försöker ansluta
    {   
        perror("listen");   
        exit(EXIT_FAILURE);   
    }   
        
    //accept the incoming connection  
    addrlen = sizeof(address);   
    puts("Waiting for connections ...");   
        
    while(TRUE)   
    {   
        //clear the socket set  
        FD_ZERO(&readfds);   
    
        //add master socket to set  
        FD_SET(master_socket, &readfds);   
        max_sd = master_socket;   
            
        //add child sockets to set  
        for ( i = 0 ; i < max_clients ; i++)   
        {   
            //socket descriptor  
            sd = client_socket[i];   
                
            //if valid socket descriptor then add to read list  
            if(sd > 0)   
                FD_SET( sd , &readfds);   
                
            //highest file descriptor number, need it for the select function  
            if(sd > max_sd)   
                max_sd = sd;   
        }
  
    
        //wait for an activity on one of the sockets , timeout is NULL ,  
        //so wait indefinitely  
        activity = select(max_sd + 1 , &readfds , NULL , NULL , NULL);   
    
        if ((activity < 0) && (errno!=EINTR))   
        {   
            printf("select error");   
        }

        // task 2
        if (incoming_signal){
            for (i = 0; i < 30; i++){ 
                if( client_socket[i] != 0 )   
                {   
                    send(client_socket[i], "From server: Shutting down, goodbye\n", 37, 0 );   
                    close(client_socket[i]);
                }   
            }  
            exit(EXIT_SUCCESS);              
        }
        
       
        //If something happened on the master socket ,  
        //then its an incoming connection  
        if (FD_ISSET(master_socket, &readfds))   
        {   
            if ((new_socket = accept(master_socket,  
                    (struct sockaddr *)&address, (socklen_t*)&addrlen))<0)   
            {   
                perror("accept");   
                exit(EXIT_FAILURE);   
            }   
            
            //inform user of socket number - used in send and receive commands  
            printf("New connection , socket fd is %d , ip is : %s , port : %d\n" , new_socket , inet_ntoa(address.sin_addr) , ntohs 
                (address.sin_port));   
        
            //send new connection greeting message  
            if( send(new_socket, message, strlen(message), 0) != strlen(message) )   
            {   
                perror("send");   
            }   
                
            puts("Welcome message sent successfully");   
                
            //add new socket to array of sockets  
            for (i = 0; i < max_clients; i++)   
            {   
                //if position is empty  
                if( client_socket[i] == 0 )   
                {   
                    client_socket[i] = new_socket;   
                    printf("Adding to list of sockets as %d\n" , i);   
                        
                    break;   
                }   
            }   
        }   

        //else its some IO operation on some other socket 
        for (i = 0; i < max_clients; i++)   
        {  
        //  char say[] = "#say";
        //  char quit[] = "#quit";
            sd = client_socket[i];   
                
            if (FD_ISSET( sd , &readfds)) {   
                //Check if it was for closing , and also read the  
                //incoming message  
                if ((valread = read( sd , buffer, 1024)) == 0)  {   
                    //Somebody disconnected , get his details and print  
                    getpeername(sd , (struct sockaddr*)&address , (socklen_t*)&addrlen);   
                    printf("Host disconnected , ip %s , port %d \n" ,  
                        inet_ntoa(address.sin_addr) , ntohs(address.sin_port));   
                        
                    //Close the socket and mark as 0 in list for reuse  
                    close( sd );   
                    client_socket[i] = 0;
                }   

                // task 1 - #quit
                else if (strncmp(buffer, "#quit", 5) == 0){
                    //Somebody disconnected , get his details and print  
                    buffer[valread] = '\0';
                    getpeername(sd , (struct sockaddr*)&address , (socklen_t*)&addrlen);
                    printf("Host quited, ip %s , port %d \n", inet_ntoa(address.sin_addr), ntohs(address.sin_port));   
                        
                    //Close the socket and mark as 0 in list for reuse  
                    close(sd);   
                    client_socket[i] = 0;   
                }

                // task 3 - #say
                else if ((strncmp(buffer, "#say ", 5)) == 0){
                    buffer[valread] = '\0';
                    char *newbuf = buffer + 5;
                    for (int j = 0; j < max_clients; j++){ 
                        if( client_socket[j] != 0 && client_socket[j] != sd)   
                        {   
                            send(client_socket[j], newbuf, strlen(newbuf), 0);   
                        }   
                    } 
                }

                //Echo back the message that came in  
                else {   
                    //set the string terminating NULL byte on the end  
                    //of the data read 
                    buffer[valread] = '\0';   
                    send(sd , buffer , strlen(buffer) , 0 );   
                }   
            }   
        }  
    } 
    return 0;   
}   