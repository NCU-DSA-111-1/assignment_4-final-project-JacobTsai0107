#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

int sockfd = 0;
int Playernumber =-1;
int endgame =1;
struct PLAYER
{   int forClientSockfd;
    int card[17];
    int handcard;
    int outsidecard[20];
    int flower[8];
}Player;
void Showcard();

int main(int argc , char *argv[])
{

    //socket的建立

    sockfd = socket(AF_INET , SOCK_STREAM , 0);

    if (sockfd == -1){
        printf("Fail to create a socket.");
    }

    //socket的連線

    struct sockaddr_in info;
    bzero(&info,sizeof(info));//initial struct
    info.sin_family = PF_INET;//IPv4

    //localhost test
    info.sin_addr.s_addr = inet_addr("127.0.0.1");//IP addr
    info.sin_port = htons(8700);//IP port


    int err = connect(sockfd,(struct sockaddr *)&info,sizeof(info));
    if(err==-1){
        printf("Connection error");
    }
    else{
        printf("linked successful\n");
        //Send a message to server
        char a[100] = "";
        //pthread_create(&t1, NULL, Rcv,a);
        recv(sockfd,a,sizeof(a),0);//welcome
        printf("%s",a);
        recv(sockfd,a,sizeof(a),0);//Player num
        printf("%s",a);
        recv(sockfd,a,sizeof(a),0);//num
        printf("%s\n",a);
        recv(sockfd,a,sizeof(a),0);//Dice
        printf("%s",a);
        do{ 
            char r[100] = "";
            char s[100] = "";
            recv(sockfd,r,sizeof(r),0);//輸入指令
            sleep(0.05);
            switch(r[0]){
                case 'S'://Showcard
                    Showcard();
                break;
                case 'W'://endgame
                    recv(sockfd,r,sizeof(r),0);//num
                    printf("%s",r);
                    endgame = 0;
                break;
                case 'L'://waste game
                    recv(sockfd,r,sizeof(r),0);//num
                    printf("%s",r);
                break;
                case 'H'://someone Hit the card
                    recv(sockfd,r,sizeof(r),0);//num
                    printf("%s",r);
                break;
                case 'Y'://yourturn
                    recv(sockfd,r,sizeof(r),0);//num
                    printf("%s",r);
                    scanf("%s",s);
                    getchar();
                    sleep(0.1);
                    send(sockfd,s,sizeof(s),0);
                break;
                case 'E'://eat card
                    recv(sockfd,s,sizeof(s),0);//num
                    printf("%s",r);
                    scanf("%s",s);
                    getchar();
                    sleep(0.1);
                    send(sockfd,s,sizeof(s),0);
                    
                case 'T'://todo
                    recv(sockfd,r,sizeof(r),0);//num
                    printf("%s",r);
                    scanf("%s",s);
                    getchar();
                    sleep(0.1);
                    send(sockfd,s,sizeof(s),0);
                break;
            }
        }while(endgame);
        
        printf("rcv end");
    
        close(sockfd);
        printf("close Socket\n");
        return 0;
    }
}
void Showcard(){
     
     
    for(int i = 0;i<17;i++){//Player.card[i]
        char b[100] = "";
        recv(sockfd,b,sizeof(b),0);
        Player.card[i] = b[0]-'0';
        printf("P =  %d\n",Player.card[i]);
    }
    //printf("Showcard\n");
    for(int i = 0;i<20;i++){//Player.outsidecard[i]
        char b[100] = "";
        recv(sockfd,b,sizeof(b),0);
        Player.outsidecard[i] = b[0]-'0';
        printf("O =  %d\n",Player.outsidecard[i]);
    }
    printf("\n");
}