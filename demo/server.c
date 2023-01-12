#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#define CLEAR() printf("\033[2J")
#define MOVEDOWN(x) printf("\033[%dB", (x))
#define MOVELEFT(y) printf("\033[%dD", (y))
#define MOVETO(x,y) printf("\033[%d;%dH", (x), (y))
struct PLAYER
{   int forClientSockfd;
    int card[17];
    int handcard;
    int outsidecard[20];
    int flower[8];
};
struct PLAYER Player[4] ;


struct CARD
{
    int value;
    struct CARD *forward;
    struct CARD *next;
};
typedef struct CARD Card;
Card *first,*current,*previous,*end,*card;

int mahjongcount = 144;
bool win = false;
int maxplayernumber = 1;
int Dice();
void bubble_sort(int array[], int n);
void Showcard();
int Hu(int door[],int thrown);
void Flower(int a,int b);
int sockfd = 0,forClientSockfd = 0;
static void * Play();
int countplayernum = 0;
char test[20] = "test";
int mahjong[36] = { 0 };//0~8 萬,9~17 索,18~26 筒, 26~33 大字, 34~35 花牌 
    
int main(int argc , char *argv[]){
         srand(time(NULL));
    for(int i =0;i<4;i++){//初始化 牌
        for(int j=0;j<20;j++){
            Player[i].card[j] = -1;
            Player[i].flower[j] = -1;
            Player[i].outsidecard[j] = -1;
        }
        Player[i].handcard = 16;
        Player[i].forClientSockfd = -1;

    }

    for(int i = 0;i < 144; i++){
        int x = 0;
        int a = 0;
        do{
            x = random()%36;
        }while(mahjong[x] >=4 );
        mahjong[x] ++;
        current = (Card*)malloc(sizeof(Card));
        if(i == 0){
            first = current;
            previous = current;
        }
        else previous->next = current;
        current->value = x;
        current->forward = previous;
        current->next = NULL;
        previous = current;     
    }
    current->next = first;


    //socket的建立
    char inputBuffer[256] = {};
    char message[] = {"Welcome to Mahjong Game!\nif all people are done,game wiil start'\n"};
    sockfd = socket(AF_INET , SOCK_STREAM , 0);
    if (sockfd == -1){
        printf("Fail to create a socket.");
    }

    //socket的連線
    struct sockaddr_in serverInfo,clientInfo;
    int addrlen = sizeof(clientInfo);
    bzero(&serverInfo,sizeof(serverInfo));

    serverInfo.sin_family = PF_INET;
    serverInfo.sin_addr.s_addr = INADDR_ANY;
    serverInfo.sin_port = htons(8700);
    bind(sockfd,(struct sockaddr *)&serverInfo,sizeof(serverInfo));
    listen(sockfd,4);

    while(1){
        
        Player[countplayernum].forClientSockfd = accept(sockfd,(struct sockaddr*) &clientInfo, &addrlen);
        char a[1] = {0};
        send(Player[countplayernum].forClientSockfd,message,sizeof(message),0);
        //s = pthread_create(&t1, NULL, Play,a);
        countplayernum ++;
        if(countplayernum < maxplayernumber){
            printf("countplaternum = %d\n",countplayernum);
        }
        else {
            Play();
            /*int i = Dice();
            sleep(5);
            Showcard();*/
        }
        
    }
    return 0;
}




static void * Play(){
    char wtellplayer[50] = "You are Player ";
    char wplayernum[10]  = "";
    
    for(int i = 0;i<maxplayernumber;i++){
        sprintf(wplayernum,"%d",i);
        sleep(0.15);
        send(Player[i].forClientSockfd,wtellplayer,sizeof(wtellplayer),0);
        sleep(0.15);
        send(Player[i].forClientSockfd,wplayernum,sizeof(wplayernum),0);
    }
    int abc;
    //砌牌

    int dice = Dice();
    //scanf("%d",&abc);
    sleep(1);
    

    //抓莊 抓牌
    int start = 36*(dice%4)-2*dice;
    if(start < 0)start = start + 144;
    current = first;
    for(int i = 0;i<144-start;i++){
        current = current ->next;
    }
    first = current;
    end = first->forward;
    for(int i = 0;i<4;i++){ //4round
        for(int j = 0 ;j<4;j++){ //4player
        int a = 4*i;
            for(int x = 0;x < 4;x++,a++){//4cards
            Player[j].card[a] = current->value;
            current = current->next;
            mahjongcount --;            
            }
        }
    }
    Player[0].card[16] = current->value;
    current = current->next;

    
    Showcard(countplayernum);
    sleep(0.5);
    //補花
    int flower = 0;
    do{
        flower = 0;
        for(int i = 0;i<4;i++){
            for(int j = 0;j<17;j++){
                if(Player[i].card[j]>33){
                    Flower(Player[i].card[j],i);
                    Player[i].card[j] = end->value;
                    end = end->forward;
                    mahjongcount --;
                    flower = 1;
                }
            }
        }
    }while(flower == 1);
    for(int i = 0;i<4;i++){
        if(i == 0)bubble_sort(Player[i].card,17);
        else bubble_sort(Player[i].card,16);
    }
    //--------抓牌結束-------//
    int cardnum;
    int playercount = 0;
    int lastcard = 0;
    do{
        int Eat[4][4];
        for(int i = 0;i<4;i++)for(int j=0;j<4;j++)Eat[i][j] = -1;
        if(playercount !=0){
            int playeraction[4] = {-1};
            for(int i = 0;i <3;i++){
                int acterror = 0;
                int pongcheck = 0;
                int eatcheck = 0;
                int hucheck = 0;
                for(int j=0;j<Player[(playercount+i)%4].handcard;j++){
                    if(Player[(playercount+i)%4].card[j]== lastcard)pongcheck++;//碰,明槓
                    if(lastcard <27&& i == 0){//下家吃
                        if(Player[(playercount+i)%4].card[j]/9 == lastcard/9){
                            if(0 < abs(Player[(playercount+i)%4].card[j]-lastcard) && abs(Player[(playercount+i)%4].card[j]-lastcard)<= 2 && Player[(playercount+i)%4].card[j]!=Player[(playercount+i)%4].card[j+1]){
                                if(eatcheck==1&&(Player[(playercount+i)%4].card[j]-Eat[(playercount+i)%4][eatcheck-1])>2)Eat[(playercount+i)%4][eatcheck-1] = Player[(playercount+i)%4].card[j];
                                else if(eatcheck==2&&(Player[(playercount+i)%4].card[j]-Eat[(playercount+i)%4][eatcheck-1])>2);
                                else {
                                    Eat[(playercount+i)%4][eatcheck] = Player[(playercount+i)%4].card[j];
                                    eatcheck++;
                                }
                            }
                        }
                    }
                }
                if(pongcheck >=2&&i ==0)pongcheck = 2;//不能槓上家
                else if(pongcheck <2)pongcheck =0;
                if((eatcheck == 2&&(Eat[i][eatcheck]-Eat[i][0])>2)||eatcheck <= 1)eatcheck = 0;//吃
                if(Hu(Player[(playercount+i)%4].card,lastcard) ==1)hucheck =1;
                char a[50] = "";
                sprintf(a,"Player %d 請輸入指令(0跳過 ",(playercount+i)%4);
                if(pongcheck!=0||eatcheck>=2||hucheck!=0){                    
                    if(eatcheck >= 2)strcat(a,"1吃 ");
                    if(pongcheck >= 2)strcat(a,"2碰 "); 
                    if(pongcheck==3)strcat(a,"3槓 ");
                    if(hucheck == 1)strcat(a,"4胡 ");
                    strcat(a,"):");
                
                    char wtodo[] ="T";
                    char wrtodo[10]="-1";
                    do{ //big problem
                        
                        acterror = 0;
                        send(Player[(playercount+i)%4].forClientSockfd,wtodo,sizeof(wtodo),0);
                        sleep(0.15);
                        printf("%s",a);
                        send(Player[(playercount+i)%4].forClientSockfd,a,sizeof(a),0);
                        sleep(0.1);
                        recv(Player[(playercount+i)%4].forClientSockfd,wrtodo,sizeof(wrtodo),0);
                        playeraction[(playercount+i)%4] = wrtodo[0]-'0';
                        printf("wrtodo = %s P= %d\n",wrtodo,playeraction[(playercount+i)%4]);
                        if(eatcheck == 0&& playeraction[(playercount+i)%4] ==1)acterror = 1;
                        else if(pongcheck != 2&& playeraction[(playercount+i)%4] ==2)acterror = 1;
                        else if(pongcheck!=3&& playeraction[(playercount+i)%4] ==3)acterror = 1;
                        if(hucheck == 0&& playeraction[(playercount+i)%4] ==4)acterror = 1;
                    }while(playeraction[(playercount+i)%4] > 4||playeraction[(playercount+i)%4]<0||acterror == 1);
                }
            }

            int temp = 0;//比較指令優先級
            int compare = -1;
            for(int i = 0;i <4;i++)if(playeraction[i] > compare){
                temp = i;
                compare = playeraction[i];
            }

            if(playeraction[temp] ==0){
                //摸牌
                Player[playercount].card[Player[playercount].handcard] = current->value;
                current = current->next;
                mahjongcount --;
                //補花
                while(Player[playercount].card[Player[playercount].handcard]>33){//
                    Flower(Player[playercount].card[Player[playercount].handcard] ,playercount);
                    Player[playercount].card[Player[playercount].handcard] = end->value;
                    end = end->forward;
                    mahjongcount --;
                    Showcard(countplayernum);
                }
            }
            else if(playeraction[temp] == 1){//吃牌
                int c1 = -1,a = 0;
                char word[50] = "";
               
                sprintf(word,"Player %d 請輸入用來吃牌的組合: 1:%d %d",temp,Eat[temp][0],Eat[temp][1]);
                if(Eat[temp][2] != -1){  
                    a = 1;
                    if(Eat[temp][2]!=-1){
                        sprintf(word," 2:%d %d",Eat[temp][1],Eat[temp][2]);
                        a = 2;
                    }
                    if(Eat[temp][3]!=-1){
                        sprintf(word," 3:%d %d",Eat[temp][2],Eat[temp][3]);
                        a = 3;
                    }   
                    char watecard[] = "E";
                    char wordc1[10] = "";
                    do{
                         
                        memset(wordc1,0,10);
                        send(Player[temp].forClientSockfd,watecard,sizeof(watecard),0);
                        sleep(0.15);
                        printf("%s",word);
                        send(Player[temp].forClientSockfd,word,sizeof(word),0);
                        recv(Player[temp].forClientSockfd,wordc1,sizeof(wordc1),0);
                        c1 = wordc1[0];
                    }while(c1<1||c1>a);
                    c1--;
                }
                int j = 0;
                while(Player[temp].outsidecard[j] != -1)j++;
                Player[temp].outsidecard[j] = Eat[temp][c1];
                Player[temp].outsidecard[j+1] = lastcard;
                Player[temp].outsidecard[j+2] = Eat[temp][c1+1];
                for(int i = 0,b = 0;i<Player[temp].handcard&&b<2;i++){
                    if(Player[temp].card[i] == Eat[temp][c1]){
                        Player[temp].card[i] = Player[temp].card[Player[temp].handcard-b-1];
                        Player[temp].card[Player[temp].handcard-b-1] = -1;
                        c1++;
                        b++;
                    };
                }
                Player[temp].handcard =Player[temp].handcard -3;
                playercount = temp;
                bubble_sort(Player[temp].card,Player[temp].handcard+1);
            }
            else if(playeraction[temp] == 2){//碰牌
                int j = 0;
                while(Player[temp].outsidecard[j] != -1)j++;
                Player[temp].outsidecard[j] = lastcard;
                Player[temp].outsidecard[j+1] = lastcard;
                Player[temp].outsidecard[j+2] = lastcard;
                for(int i = 0,b = 0;i<Player[temp].handcard&&b<2;i++){
                    if(Player[temp].card[i] == lastcard){
                        Player[temp].card[i] = Player[temp].card[Player[temp].handcard-b-1];
                        Player[temp].card[Player[temp].handcard-b-1] = -1;
                        b++;
                    }
                }
                Player[temp].handcard  = Player[temp].handcard-3;
                Player[temp].card[Player[temp].handcard+1] =-1;
                playercount = temp;
                bubble_sort(Player[temp].card,Player[temp].handcard+1);
            }
            else if(playeraction[temp] == 3){//槓牌
                int j = 0;
                while(Player[temp].outsidecard[j] != -1)j++;
                Player[temp].outsidecard[j] = lastcard;
                Player[temp].outsidecard[j+1] = lastcard;
                Player[temp].outsidecard[j+2] = lastcard;
                Player[temp].outsidecard[j+3] = lastcard;
                for(int i = 0,b = 0;i<Player[temp].handcard&&b<3;i++){
                    if(Player[temp].card[i] == lastcard){
                        Player[temp].card[i] = Player[temp].card[Player[temp].handcard-b-1];
                        Player[temp].card[Player[temp].handcard-b-1] = -1;
                        b++;
                    };
                }
                Player[temp].handcard=Player[temp].handcard -3;
                Player[temp].card[Player[temp].handcard] = -1;
                playercount = temp;
                do{
                    Player[playercount].card[Player[playercount].handcard] = end->value;
                    if(Player[playercount].card[Player[playercount].handcard]>33)Flower(playercount,Player[playercount].card[Player[playercount].handcard]);
                    end = end->forward;
                    mahjongcount --;
                    Showcard(countplayernum);
                }while(Player[playercount].card[Player[playercount].handcard]>33);
                bubble_sort(Player[temp].card,Player[temp].handcard+1);
            }
            else if(playeraction[temp]==4){//胡牌
                Player[temp].card[Player[temp].handcard] = lastcard;
                win = true;
                break;
            }            
        }
        Showcard(countplayernum);
        int gangcheck = 0;
        int hucheck = 0;
        hit:
        gangcheck = 0;
        hucheck = 0;
        int gangcard = -1;
        char a[50] ="";
        sprintf(a,"Player %d 請輸入指令(要打出的牌01 ~ %d",playercount,(Player[playercount].handcard+1));
        for(int j =0;j<Player[playercount].handcard;j++){//暗槓
            if(Player[playercount].card[j] == Player[playercount].card[j+1]){
                gangcheck++;
                gangcard = j;
            }
            else if(gangcheck == 2&&Player[playercount].card[j]==Player[playercount].card[Player[playercount].handcard])gangcheck = 4;
            else gangcheck =0;
        }
        int j = 0;
        while(j<18){//明槓
            if(Player[playercount].outsidecard[j] == Player[playercount].outsidecard[j+1]&&Player[playercount].outsidecard[j+1]==Player[playercount].outsidecard[j+2]&&Player[playercount].outsidecard[j+2]!=-1){
                for(int i = 0;i <= Player[playercount].handcard;i++){
                    if(Player[playercount].outsidecard[j] == Player[playercount].card[i]){
                        gangcard = i;
                        gangcheck =5;
                        break;
                    }
                }
            }
            j++;
        }
        if(gangcheck>=3)strcat(a," 0槓");
        if(Hu(Player[playercount].card,-1)==1){
            strcat(a," -1胡");
            hucheck =1;
        }
        strcat(a,"):\n");
        char wyourturn[] = "Y";
        char wordcardnum[10] = "-1";
        do{//big problem

            send(Player[playercount].forClientSockfd,wyourturn,sizeof(wyourturn),0);
            sleep(0.15);
            printf("%s",a);
            send(Player[playercount].forClientSockfd,a,sizeof(a),0);
            sleep(0.15);
            recv(Player[playercount].forClientSockfd,wordcardnum,sizeof(wordcardnum),0);
            cardnum = (wordcardnum[0]-'0')*10+(wordcardnum[1]-'0');
            printf("w = %s cardnum = %d\n",wordcardnum,cardnum);
            if(gangcheck < 3&& cardnum == 0)cardnum=-3;
            else if(hucheck != 1 && cardnum ==-1)cardnum=-3;
            cardnum = cardnum-1;
        }while(cardnum>Player[playercount].handcard||cardnum<-2);
        if(cardnum == -1){//槓
            int j = 0,b = 0;    
            if(gangcheck == 3){
                for(int i = 0;i<Player[playercount].handcard&&b<3;i++){//暗槓
                        if(Player[playercount].card[i] == Player[playercount].card[gangcard]){
                            Player[playercount].card[i] = Player[playercount].card[Player[playercount].handcard-b];
                            Player[playercount].card[Player[playercount].handcard-b] = -1;
                            b++;
                        };
                }
                while(Player[playercount].outsidecard[j] != -1&&Player[playercount].outsidecard[j]!=Player[playercount].card[gangcard]&&Player[playercount].outsidecard[j+1]!=Player[playercount].card[gangcard])j++;
                if(Player[playercount].outsidecard[j] == -1){//暗槓
                    Player[playercount].outsidecard[j] = Player[playercount].card[gangcard];
                    Player[playercount].outsidecard[j+1] = Player[playercount].card[gangcard];
                    Player[playercount].outsidecard[j+2] = Player[playercount].card[gangcard];
                    Player[playercount].outsidecard[j+3] = Player[playercount].card[gangcard];
                    Player[playercount].handcard =Player[playercount].handcard -3;
                }
            }
            else if(gangcheck == 4){
                for(int i = 0;i<Player[playercount].handcard&&b<2;i++){//暗槓
                        if(Player[playercount].card[i] == Player[playercount].card[Player[playercount].handcard]){
                            Player[playercount].card[i] = Player[playercount].card[Player[playercount].handcard-b];
                            Player[playercount].card[Player[playercount].handcard-b] = -1;
                            b++;
                        };
                }
                while(Player[playercount].outsidecard[j] != -1)j++;
                if(Player[playercount].outsidecard[j] == -1){//暗槓
                    Player[playercount].outsidecard[j] = Player[playercount].card[Player[playercount].handcard];
                    Player[playercount].outsidecard[j+1] = Player[playercount].card[Player[playercount].handcard];
                    Player[playercount].outsidecard[j+2] = Player[playercount].card[Player[playercount].handcard];
                    Player[playercount].outsidecard[j+3] = Player[playercount].card[Player[playercount].handcard];
                }
                Player[playercount].handcard =Player[playercount].handcard -3;
            }
            else if(gangcheck == 5){
                while(Player[playercount].outsidecard[j]!=Player[playercount].card[gangcard]&&Player[playercount].outsidecard[j+1]!=Player[playercount].card[gangcard])j++;
                if(Player[playercount].outsidecard[j]==Player[playercount].card[gangcard]&&Player[playercount].outsidecard[j+1]==Player[playercount].card[gangcard]){
                    int temp1 = Player[playercount].card[Player[playercount].handcard];
                    int temp2 = -1;
                    j=j+3;
                    while(temp1 != -1){
                        temp2 = Player[playercount].outsidecard[j];
                        Player[playercount].outsidecard[j] = temp1;
                        temp1 = temp2;
                        j++;
                    }
                }
                Player[playercount].handcard =Player[playercount].handcard -1;
            }
            
            bubble_sort(Player[playercount].card,Player[playercount].handcard+1);
            do{
                Player[playercount].card[Player[playercount].handcard] = end->value;
                if(Player[playercount].card[Player[playercount].handcard]>33)Flower(playercount,Player[playercount].card[Player[playercount].handcard]);
                end = end->forward;
                mahjongcount --;
                Showcard(countplayernum);
            }while(Player[playercount].card[Player[playercount].handcard]>33);
            goto hit;
        }
        else if(cardnum == -2){
            Player[playercount].card[Player[playercount].handcard] = lastcard;
            win = true;
            break;
        }
        lastcard = Player[playercount].card[cardnum];
        Player[playercount].card[cardnum] = Player[playercount].card[Player[playercount].handcard];
        Player[playercount].card[Player[playercount].handcard] = -1;
        bubble_sort(Player[playercount].card,Player[playercount].handcard);
        Showcard(countplayernum);
        char wordhit[50] = "";
        char whit[] = "H";
        sleep(0.15);

        for(int i=0;i<maxplayernumber;i++){
            sleep(0.15);
            send(Player[i].forClientSockfd,whit,sizeof(whit),0);
        }
        sprintf(wordhit, "Player %d 打出了 %d\n", playercount, lastcard);
        sleep(0.15);
        for(int i=0;i<maxplayernumber;i++){
            sleep(0.15); 
            send(Player[i].forClientSockfd,wordhit,sizeof(wordhit),0);
        }
        printf("%s",wordhit);
        playercount++;
        if(playercount == 4)playercount =0;
        sleep(1);
    }while(win == false && mahjongcount > 16);

    
    char wordendgame[50] = "";
    if(win == true){
        Showcard(countplayernum);
        char wwin[] = "W";
        for(int i=0;i<maxplayernumber;i++){
            sleep(0.15);
            send(Player[i].forClientSockfd,wwin,sizeof(wwin),0);
        }

        sprintf(wordendgame, "Player %d win", playercount);
        
        printf("%s",wordendgame);
        for(int i=0;maxplayernumber;i++){
            sleep(0.15); 
            send(Player[i].forClientSockfd,wordendgame,sizeof(wordendgame),0);
        }
    }
    if(mahjongcount <= 16){
        char wwastegame[] = "L";
        for(int i=0;i<maxplayernumber;i++){
            sleep(0.15); 
            send(Player[i].forClientSockfd,wwastegame,sizeof(wwastegame),0);
        }

        strcat(wordendgame,"Game is waste");
        for(int i=0;maxplayernumber;i++){
            sleep(0.15); 
            send(Player[i].forClientSockfd,wordendgame,sizeof(wordendgame),0);
        }
        printf("%s",wordendgame);
    }

    
    return 0;

}


int Dice(){
    int dice = 0;
    char wordDice[100] = "";
    strcat(wordDice,"Dice:");
    char wordx[2] = "";
    for(int i = 0;i < 3;i++){
        int x = 6 - random()%6;
        sprintf(wordx, "%c", x+'0');
        dice = dice + x;
        strcat(wordDice,wordx);
        strcat(wordDice," ");   
    }
    strcat(wordDice,"\n");
    for(int i = 0;i<maxplayernumber;i++){
        send(Player[i].forClientSockfd,wordDice,sizeof(wordDice),0);
        sleep(0.15);
    }
    printf("%s",wordDice);
    return dice;
}

void bubble_sort(int array[], int n) {
    for (int i=0; i<n-1; i++) {
        for (int j=0; j<n-i-1; j++) {
            if (array[j] > array[j+1]) {
                int temp = array[j];
                array[j] = array[j+1];
                array[j+1] = temp;
            }
        }
    }
}

void Showcard(){
    char wshowcard[]="S";
    printf("           1    2    3    4    5    6    7    8    9    10   11   12   13   14   15   16   17  門前\n");
    for(int i = 0;i<4;i++){
        printf("Player %d :",i);
        send(Player[i].forClientSockfd,wshowcard,sizeof(wshowcard),0);
        sleep(0.5);
        char wPlayercard[10] ="";
        for(int j =0 ;j<17;j++){
            
            memset(wPlayercard,0,10);
            wPlayercard[0] = Player[i].card[j]+'0';
            send(Player[i].forClientSockfd,wPlayercard,sizeof(wPlayercard),0);
            //printf("w= %s P= %d i = %d\n",wPlayercard,Player[i].card[j],j);
            sleep(0.15);
            switch(Player[i].card[j]){
                case -1:
                    printf("     ");
                break;
                case 0 ... 8:
                    printf("%2d萬 ",Player[i].card[j]+1);
                break;
                case 9 ... 17:
                    printf("%2d索 ",Player[i].card[j]-8);
                break;
                case 18 ... 26:
                    printf("%2d筒 ",Player[i].card[j]-17);
                break;
                case 27:
                    printf("東風 ");
                break;
                case 28:
                    printf("南風 ");
                break;
                case 29:
                    printf("西風 ");
                break;
                case 30:
                    printf("北風 ");
                break;
                case 31:
                    printf("紅中 ");
                break;
                case 32:
                    printf("青發 ");
                break;
                case 33:
                    printf("白板 ");
                break;
                case 34 ... 35:
                    printf("花牌 ");
                break;
                default:
                break;
            
            }
        }
        char wPlayeroutsidecard[2] ="";
        for(int j = 0, a = 0;j< 20;j++,a++){
            
            memset(wPlayeroutsidecard,0,2);
            wPlayeroutsidecard[0] = Player[i].outsidecard[j]+'0';
            send(Player[i].forClientSockfd,wPlayeroutsidecard,sizeof(wPlayeroutsidecard),0);
            //printf("w= %s P= %d i = %d\n",wPlayercard,Player[i].card[j],j);
            sleep(0.15);
            switch(Player[i].outsidecard[j]){
                case 0 ... 8:
                    printf("%2d萬",Player[i].outsidecard[j]+1);
                break;
                case 9 ... 17:
                    printf("%2d索",Player[i].outsidecard[j]-8);
                break;
                case 18 ... 26:
                    printf("%2d筒",Player[i].outsidecard[j]-17);
                break;
                case 27:
                    printf("🀀");
                break;
                case 28:
                    printf("南風");
                break;
                case 29:
                    printf("西風");
                break;
                case 30:
                    printf("北風");
                break;
                case 31:
                    printf("紅中");
                break;
                case 32:
                    printf("青發");
                break;
                case 33:
                    printf("白板");
                break;
                case 34 ... 35:
                    printf("花牌");
                break;
                default:
                break;
            }
        }
        //char whandcard[1];
        //whandcard[0] = Player[i].handcard;
        printf("\n");
        //send(Player[i].forClientSockfd,Player[i].outsidecard,sizeof(Player[i].outsidecard),0);
        //send(Player[i].forClientSockfd,Player[i].flower,sizeof(Player[i].flower),0);
        //send(Player[i].forClientSockfd,whandcard,sizeof(whandcard),0);
    }
    printf("\n");
    
}
/*void Showcard(int players){
    int setup=2;
    int check=0;
    CLEAR();
    MOVETO(1,1);
    printf("Player %d :",players);
    for(int j =0 ;j<17;j++){
        MOVETO(19,2);
        switch(Player[players].card[j]){
            case -1:
                printf(" ");
            break;
            case 0:
                printf("🀇");
            break;
            case 1:
                printf("🀈");
            break;
            case 2:
                printf("🀉");
            break;
            case 3:
                printf("🀊");
            break;
            case 4:
                printf("🀋");
            break;
            case 5:
                printf("🀌");
            break;
            case 6:
                printf("🀍");
            break;
            case 7:
                printf("🀎");
            break;
            case 8:
                printf("🀏");
            break;
            case 9:
                printf("🀙");
            break;
            case 10:
                printf("🀚");
            break;
            case 11:
                printf("🀛");
            break;
            case 12:
                printf("🀜");
            break;
            case 13:
                printf("🀝");
            break;
            case 14:
                printf("🀞");
            break;
            case 15:
                printf("🀟");
            break;
            case 16:
                printf("🀠");
            break;
            case 17:
                printf("🀡");
            break;
            case 18:
                printf("🀐");
            break;
            case 19:
                printf("🀑");
            break;
            case 20:
                printf("🀒");
            break;
            case 21:
                printf("🀓");
            break;
            case 22:
                printf("🀔");
            break;            
            case 23:
                printf("🀕");
            break;
            case 24:
                printf("🀖");
            break;    
            case 25:
                printf("🀗");
            break;
            case 26:
                printf("🀘");
            break;
            case 27:
                printf("🀀");
            break;
            case 28:
                printf("🀁");
            break;
            case 29:
                printf("🀂");
            break;
            case 30:
                printf("🀃");
            break;
            case 31:
                printf("🀄");
            break;
            case 32:
                printf("🀅");
            break;
            case 33:
                printf("🀆");
            break;
            case 34 ... 35:
                printf("🀫");
            break;
            default:
                printf("🀫");
            break;
            
        }
    }
    for(int i = 0;i<4;i++){
        for(int j = 0;j< 20;j++){
            if((abs(players-i)%4)==0){
                MOVETO(17,2);
            }
            if((abs(players-i)%4)==1){
                MOVETO(9,16);
            }
            if((abs(players-i)%4)==2){
                MOVETO(2,9);
            }
            if((abs(players-i)%4)==3){
                MOVETO(9,3);
            }
            switch(Player[i].outsidecard[j]){
                 case 0:
                    printf("🀇");
                break;
                case 1:
                    printf("🀈");
                break;
                case 2:
                    printf("🀉");
                break;
                case 3:
                    printf("🀊");
                break;
                case 4:
                    printf("🀋");
                break;
                case 5:
                    printf("🀌");
                break;
                case 6:
                    printf("🀍");
                break;
                case 7:
                    printf("🀎");
                break;
                case 8:
                    printf("🀏");
                break;
                case 9:
                    printf("🀙");
                break;
                case 10:
                    printf("🀚");
                break;
                case 11:
                    printf("🀛");
                break;
                case 12:
                    printf("🀜");
                break;
                case 13:
                    printf("🀝");
                break;
                case 14:
                    printf("🀞");
                break;
                case 15:
                    printf("🀟");
                break;
                case 16:
                    printf("🀠");
                break;
                case 17:
                    printf("🀡");
                break;
                case 18:
                    printf("🀐");
                break;
                case 19:
                    printf("🀑");
                break;
                case 20:
                    printf("🀒");
                break;
                case 21:
                    printf("🀓");
                break;
                case 22:
                    printf("🀔");
                break;
                case 23:
                    printf("🀕");
                break;
                case 24:
                    printf("🀖");
                break;
                case 25:
                    printf("🀗");
                break;
                case 26:
                    printf("🀘");
                break;
                case 27:
                    printf("🀀");
                break;
                case 28:
                    printf("🀁");
                break;
                case 29:
                    printf("🀂");
                break;
                case 30:
                    printf("🀃");
                break;
                case 31:
                    printf("🀄");
                break;
                case 32:
                    printf("🀅");
                break;
                case 33:
                    printf("🀆");
                break;
                case 34 ... 35:
                    printf("🀫");
                break;
                default:
                    printf("🀫");
                break;
            }
            if(j%3 ==setup&&(abs(players-i)%4)!=0){
                if((Player[i].outsidecard[j]==Player[i].outsidecard[j+1])&&(Player[i].outsidecard[j+2]-Player[i].outsidecard[j+1]!=1||Player[i].outsidecard[j+4]-Player[i].outsidecard[j+2]==2)){
                    setup++;
                    setup=setup%3;
                    check=1;
                }
                else{
                    MOVEDOWN(1);
                    if(check){
                        MOVELEFT(4);
                        check=0;
                    }
                    else{
                        MOVELEFT(3);
                    }
                }
            }
        }
    }
    MOVETO(22,1);
}*/

void Flower(int a,int b){
    int i = 0;
    while(Player[b].flower[i]!=0)i++;
    Player[b].flower[i] = a;
}

int Hu(int door[],int thrown){
    int list[4][9]={0};
    int eyes[8]={0};
    int row;
    int col;
    int checktime = 0;
    int nohu = 0;
    int win = 0;
    if(thrown!=-1){
       row = thrown/9;
       col = thrown%9;
       list[row][col] += 1;
    }
    for(int i = 0; door[i]!=-1&&i<17;i++){
        row = door[i]/9;
        col = door[i]%9;
        list[row][col] += 1;
    }
    list[3][7]=0;
    list[3][8]=0;
    for(int i=0;i<=3;i++){
        for(int j=0;j<=8;j++){
            if(list[i][j]>=2){
               eyes[checktime] = i*9+j;
               checktime += 1;
            }
        }
    }
    for(checktime;checktime>0;checktime--){
        int x = eyes[checktime-1]/9;
        int y = eyes[checktime-1]%9;
        list[x][y] -= 2;
        for(int i=0;i<=3;i++){
            for(int j=0;j<=8;j++){
                if(list[i][j]>=3){
                    list[i][j]-=3;
                }
                if(list[i][j]==1&&row<3){
                    if(list[i][j+1]>=1&&list[i][j+2]>=1&&j>=0&&j<=6){
                       list[i][j] -= 1;
                       list[i][j+1] -= 1;
                       list[i][j+2] -= 1;
                    }
                    else if(list[i][j+1]>=1&&list[i][j-1]>=1&&j>=1&&j<=7){
                       list[i][j] -= 1;
                       list[i][j+1] -= 1;
                       list[i][j-1] -= 1;
                    }
                    else if(list[i][j-2]>=1&&list[i][j-1]>=1&&j>=2&&j<=8){
                       list[i][j] -= 1;
                       list[i][j-2] -= 1;
                       list[i][j-1] -= 1;
                    }
                    else nohu = 1;
                }
            }
        }
        if(nohu==0){
            for(int i=0;i<=3;i++){
                for(int j=0;j<=8;j++){
                    if(list[i][j]!=0){
                    nohu = 1;
                    }
                }
            }            
        }
        if(nohu == 0){
            win = 1;
            break;
        }
        if(nohu == 1){
            if(thrown!=-1){
                row = thrown/9;
                col = thrown%9;
                list[row][col] += 1;
            }
            for(int i = 0; door[i]!=-1&&i<17;i++){
                row = door[i]/9;
                col = door[i]%9;
                list[row][col] += 1;
                nohu = 0;
            }
            list[3][7]=0;
            list[3][8]=0;
        }
    }
    return win;
}