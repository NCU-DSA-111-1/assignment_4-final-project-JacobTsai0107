#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#define CLEAR() printf("\033[2J")
#define MOVEDOWN(x) printf("\033[%dB", (x))
#define MOVELEFT(y) printf("\033[%dD", (y))
#define MOVETO(x,y) printf("\033[%d;%dH", (x), (y))
struct PLAYER
{   int IP;
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

int Dice();
void bubble_sort(int array[], int n);
void Showcard(int players);
int Hu(int door[],int thrown);
void Flower(int a,int b);
//int calculate(int inside[], int outside[], int outfl[],  int thrown);


int main(){
    int mahjong[36] = { 0 };//0~8 萬,9~17 索,18~26 筒, 26~33 大字, 34~35 花牌 
    for(int i =0;i<4;i++){//初始化 牌
        for(int j=0;j<20;j++){
            Player[i].card[j] = -1;
            Player[i].flower[j] = -1;
            Player[i].outsidecard[j] = -1;
        }
        Player[i].handcard = 16;
    }

    //砌牌
    srand(time(NULL));
    for(int i = 0;i < 144; i++){//make cards random, using doubly circular linked list to connect them
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

    //抓莊 抓牌
    int dice = Dice();
    int start = 36*(dice%4)-2*dice;
    if(start < 0)start = start + 144;
    current = first;
    for(int i = 0;i<144-start;i++){
        current = current ->next;
    }
    first = current;
    end = first->forward;
    for(int i = 0;i<4;i++){ //4rounds
        for(int j = 0 ;j<4;j++){ //4players
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
    Showcard(0);
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
        if(i == 0)bubble_sort(Player[i].card,17);//sorted cards
        else bubble_sort(Player[i].card,16);
    }
    //--------抓牌結束-------//
    int cardnum;
    int playercount = 0;
    int lastcard = 0;
    do{//game operates
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
                char a[50] = {""};
                char b[1];
                b[0] = (playercount+i)%4;
                strcat(a,"Player ");
                strcat(a,b);
                strcat(a," 請輸入指令(0跳過 ");
                if(pongcheck!=0||eatcheck>=2||hucheck!=0){//checklist announced if corresponded                     
                    if(eatcheck >= 2)strcat(a,"1吃 ");
                    if(pongcheck >= 2)strcat(a,"2碰 "); 
                    if(pongcheck==3)strcat(a,"3槓 ");
                    if(hucheck == 1)strcat(a,"4胡 ");
                    strcat(a,"):");
                    do{ 
                        acterror = 0;
                        printf("%s",a);
                        scanf("%d",&playeraction[(playercount+i)%4]);
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
                Player[playercount%4].card[Player[playercount%4].handcard] = current->value;
                current = current->next;
                mahjongcount --;
                //補花
                while(Player[playercount%4].card[Player[playercount%4].handcard]>33){//
                    Flower(Player[playercount%4].card[Player[playercount%4].handcard] ,playercount%4);
                    Player[playercount%4].card[Player[playercount%4].handcard] = end->value;
                    end = end->forward;
                    mahjongcount --;
                    Showcard(playercount%4);
                }
            }
            //any action will put cards into the front door(outsidecard), removing from the handcardlist(card).
            else if(playeraction[temp] == 1){//吃牌
                int c1 = -1,a = 0;
                if(Eat[temp][2] != -1){     
                    do{
                        printf("Player %d請輸入用來吃牌的組合:",temp);
                        printf("1:%d %d ",Eat[temp][0],Eat[temp][1]);
                        a = 1;
                        if(Eat[temp][2]!=-1){
                            printf("2:%d %d ",Eat[temp][1],Eat[temp][2]);
                            a = 2;
                        }
                        if(Eat[temp][3]!=-1){
                            printf("3:%d %d ",Eat[temp][2],Eat[temp][3]);
                            a = 3;
                        }
                        scanf("%d",&c1);
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
                    };
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
                    Showcard(playercount%4);
                }while(Player[playercount%4].card[Player[playercount].handcard]>33);
                bubble_sort(Player[temp].card,Player[temp].handcard+1);
            }
            else if(playeraction[temp]==4){//胡牌
                Player[temp].card[Player[temp].handcard] = lastcard;
                win = true;
                break;
            }            
        }
        int gangcheck = 0;
        int hucheck = 0;
        Showcard(playercount%4);
        hit:
        gangcheck = 0;
        hucheck = 0;
        int gangcard = -1;
        char a[20] ="";
        for(int j =0;j<=Player[playercount%4].handcard;j++){//暗槓
            if(Player[playercount%4].card[j] == Player[playercount%4].card[j+1]){
                gangcheck++;
                gangcard = j;
            }
            else if(gangcheck == 2&&Player[playercount%4].card[j]==Player[playercount%4].card[Player[playercount%4].handcard])gangcheck = 4;
            else gangcheck =0;
        }
        int j = 0;
        while(j<18){//明槓
            if(Player[playercount%4].outsidecard[j] == Player[playercount%4].outsidecard[j+1]&&Player[playercount%4].outsidecard[j+1]==Player[playercount%4].outsidecard[j+2]&&Player[playercount%4].outsidecard[j+2]!=-1){
                for(int i = 0;i <= Player[playercount%4].handcard;i++){
                    if(Player[playercount%4].outsidecard[j] == Player[playercount%4].card[i]){
                        gangcard = i;
                        gangcheck =5;
                        break;
                    }
                }
            }
            j++;
        }
        if(gangcheck>=3)strcat(a," 0槓");
        if(Hu(Player[playercount%4].card,-1)==1){//自摸
            strcat(a," -1胡");
            hucheck =1;
        }
        strcat(a,"):");
        do{//throw a card from the handcardlist(card)
            printf("Player %d 請輸入指令(要打出的牌1 ~ %d%s",(playercount%4),Player[playercount%4].handcard+1,a);
            scanf("%d",&cardnum);
            if(gangcheck < 3&& cardnum == 0)cardnum=-3;
            else if(hucheck != 1 && cardnum ==-1)cardnum=-3;
            cardnum = cardnum-1;
        }while(cardnum>Player[playercount%4].handcard||cardnum<-2);
        if(cardnum == -1){//槓
            int j = 0,b = 0;    
            if(gangcheck == 3){
                for(int i = 0;i<Player[playercount%4].handcard&&b<3;i++){//暗槓
                        if(Player[playercount%4].card[i] == Player[playercount%4].card[gangcard]){
                            Player[playercount%4].card[i] = Player[playercount%4].card[Player[playercount%4].handcard-b];
                            Player[playercount%4].card[Player[playercount%4].handcard-b] = -1;
                            b++;
                        };
                }
                while(Player[playercount%4].outsidecard[j] != -1&&Player[playercount%4].outsidecard[j]!=Player[playercount%4].card[gangcard]&&Player[playercount%4].outsidecard[j+1]!=Player[playercount%4].card[gangcard])j++;
                if(Player[playercount%4].outsidecard[j] == -1){//暗槓
                    Player[playercount%4].outsidecard[j] = Player[playercount%4].card[gangcard];
                    Player[playercount%4].outsidecard[j+1] = Player[playercount%4].card[gangcard];
                    Player[playercount%4].outsidecard[j+2] = Player[playercount%4].card[gangcard];
                    Player[playercount%4].outsidecard[j+3] = Player[playercount%4].card[gangcard];
                    Player[playercount%4].handcard =Player[playercount%4].handcard -3;
                }
            }
            else if(gangcheck == 4){
                for(int i = 0;i<Player[playercount%4].handcard&&b<2;i++){//暗槓
                        if(Player[playercount%4].card[i] == Player[playercount%4].card[Player[playercount%4].handcard]){
                            Player[playercount%4].card[i] = Player[playercount%4].card[Player[playercount%4].handcard-b];
                            Player[playercount%4].card[Player[playercount%4].handcard-b] = -1;
                            b++;
                        };
                }
                while(Player[playercount%4].outsidecard[j] != -1)j++;
                if(Player[playercount%4].outsidecard[j] == -1){//暗槓
                    Player[playercount%4].outsidecard[j] = Player[playercount%4].card[Player[playercount%4].handcard];
                    Player[playercount%4].outsidecard[j+1] = Player[playercount%4].card[Player[playercount%4].handcard];
                    Player[playercount%4].outsidecard[j+2] = Player[playercount%4].card[Player[playercount%4].handcard];
                    Player[playercount%4].outsidecard[j+3] = Player[playercount%4].card[Player[playercount%4].handcard];
                }
                Player[playercount%4].handcard =Player[playercount%4].handcard -3;
            }
            else if(gangcheck == 5){
                while(Player[playercount%4].outsidecard[j]!=Player[playercount%4].card[gangcard]&&Player[playercount%4].outsidecard[j+1]!=Player[playercount%4].card[gangcard])j++;
                if(Player[playercount%4].outsidecard[j]==Player[playercount%4].card[gangcard]&&Player[playercount%4].outsidecard[j+1]==Player[playercount%4].card[gangcard]){
                    int temp1 = Player[playercount%4].card[Player[playercount%4].handcard];
                    int temp2 = -1;
                    j=j+3;
                    while(temp1 != -1){
                        temp2 = Player[playercount%4].outsidecard[j];
                        Player[playercount%4].outsidecard[j] = temp1;
                        temp1 = temp2;
                        j++;
                    }
                }
                Player[playercount%4].handcard =Player[playercount%4].handcard -1;
            }
            
            bubble_sort(Player[playercount%4].card,Player[playercount%4].handcard+1);
            do{
                Player[playercount%4].card[Player[playercount%4].handcard] = end->value;
                if(Player[playercount].card[Player[playercount].handcard]>33)Flower(playercount,Player[playercount].card[Player[playercount].handcard]);
                end = end->forward;
                mahjongcount --;
                Showcard(playercount%4);
            }while(Player[playercount%4].card[Player[playercount%4].handcard]>33);
            goto hit;
        }
        else if(cardnum == -2){
            Player[playercount%4].card[Player[playercount%4].handcard] = lastcard;
            win = true;
            break;
        }
        lastcard = Player[playercount%4].card[cardnum];//the thrown away card be the lastcard, and continue the game circle
        Player[playercount%4].card[cardnum] = Player[playercount%4].card[Player[playercount%4].handcard];
        Player[playercount%4].card[Player[playercount%4].handcard] = -1;
        bubble_sort(Player[playercount%4].card,Player[playercount%4].handcard);
        Showcard(playercount%4);
        printf("Player %d 打出了 %d\n",playercount%4,lastcard);
        playercount++;

    }while(win == false && mahjongcount > 16);
    if(win == true){
        Showcard(playercount%4);
        printf("Player %d win",playercount);
    }
    if(mahjongcount <= 16)printf("you all suck");//流局

    
    return 0;

}


int Dice(){
    int dice = 0;
    for(int i = 0;i < 3;i++){
        int x = 6 - random()%6;
        dice = dice + x;
        printf("dice = %d ",x);
    }
    printf("\n");
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
void Showcard(int players){//tidy up the card display
    printf("           1    2    3    4    5    6    7    8    9    10   11   12   13   14   15   16   17  門前\n");
    for(int i = 0;i<4;i++){
        printf("Player %d :",i);
        for(int j =0 ;j<17;j++){
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
        for(int j = 0, a = 0;j< 20;j++,a++){
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

        printf("\n\n");
    }
    printf("\n");
}


void Flower(int a,int b){
    int i = 0;
    while(Player[b].flower[i]!=0)i++;
    Player[b].flower[i] = a;
}

int Hu(int door[],int thrown){//check if one wins the game
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
    for(int i = 0; door[i]!=-1&&i<17;i++){//classify cardlist
        row = door[i]/9;
        col = door[i]%9;
        list[row][col] += 1;
    }
    list[3][7]=0;
    list[3][8]=0;
    for(int i=0;i<=3;i++){
        for(int j=0;j<=8;j++){
            if(list[i][j]>=2){//find eyes first
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
            for(int j=0;j<=8;j++){//檢查順子並清除符合規則的牌
                if(list[i][j]>=3){
                    list[i][j]-=3;
                }
                if(list[i][j]==1&&row<3){//檢查刻子並清除符合規則的牌
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
        if(nohu==0){//check twice
            for(int i=0;i<=3;i++){
                for(int j=0;j<=8;j++){
                    if(list[i][j]!=0){
                    nohu = 1;
                    }
                }
            }            
        }
        if(nohu == 0){//win condition
            win = 1;
            break;
        }
        if(nohu == 1){//if there are more chances to check, refresh the checklist
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
/*int calculate(int inside[], int outside[], int outfl[],  int thrown){//算台
    int list[4][9]={0};
    int row;
    int col;
    int hucondition=0;
    int totalwan=0;
    int totalbing=0;
    int totaltail=0;
    int ponghucounter=0;
    int maximumin=0;
    int maximumout=0;
    int totalzi;
    int totalziflower=0;
    int tai=0;
    for(int i = 0; inside[i]!=-1&&i<17;i++){
        row = inside[i]/9;
        col = inside[i]%9;
        list[row][col] += 1;
        maximumin=i+1;
    }
    inside[maximumin]=-1;
    for(int i = 0; outside[i]!=-1&&i<20;i++){
        row = outside[i]/9;
        col = outside[i]%9;
        list[row][col] += 1;
        maximumout=i+1;
    }
    for(int i = 0; outfl[i]!=-1&&i<8;i++){
        row = outfl[i]/9;
        col = outfl[i]%9;
        list[row][col] += 1;
    }
    for(int i = 0;i<=3;i++){
        for(int j = 0;j<=8;j++){
            switch(i){
                case 0:
                totalwan+=list[i][j];
                case 1:
                totalbing+=list[i][j];
                case 2:
                totaltail+=list[i][j];
                case 3:
                totalziflower+=list[i][j];
            }
            if((9*i+j)<34){
                if(Hu(inside,list[i][j])==1){
                    hucondition+=1;
                }
                if(list[i][j]>=3){
                    ponghucounter+=1;
                }
            }
        }
    }
    totalzi = totalziflower-(list[3][7]+list[3][8]);
    if(thrown==-1){
        tai+=1;
    }
    if(outside[0]==-1){
        tai+=1;
        if(thrown==-1){
            tai+=1;
        }
    }
    if(hucondition==1){
        tai+=1;
    }
    if(list[3][7]==4||list[3][8]==4){
        tai+=2;
        if(list[3][7]==4&&list[3][8]==4){
            tai+=6;
        }
    }
    if(ponghucounter==5){
        tai+=4;
        if(thrown==-1){
            tai+=8;
        }
        if(maximumout==3||maximumout==4){
            tai+=5;
        }
        if(maximumout>=6||maximumout<=8){
            tai+=2;
        }
    }
    if(thrown!=-1&&totalziflower==0&&outside[0]!=-1){
        tai+=2;
    }
    if((totalbing==0&&totaltail==0)||(totalbing==0&&totalwan==0)||(totalwan==0&&totaltail==0)){
        if(totalzi==0){
            tai+=8;
        }
        else{
            tai+=4;
            if(totalbing==0&&totaltail==0&&totalwan==0){
                tai+=12;
            }
        }
    }
    if(list[3][4]>=3||list[3][5]>=3||list[3][6]>=3){
        tai+=1;
        if((list[3][4]>=3&&list[3][5]>=3)||(list[3][4]>=3&&list[3][6]>=3)||(list[3][6]>=3&&list[3][5]>=3)){
            tai+=1;
            if(list[3][4]>=2&&list[3][5]>=2&&list[3][6]>=2){
                tai+=2;
            }
            if(list[3][4]>=3&&list[3][5]>=3&&list[3][6]>=3){
                tai+=4;
            }
        }
    }
    if(list[3][0]>=2&&list[3][1]>=2&&list[3][2]>=2&&list[3][3]>=2){
        tai+=8;
    }
    if(list[3][0]>=3&&list[3][1]>=3&&list[3][2]>=3&&list[3][3]>=3){
        tai+=16;
    }
    
}*/