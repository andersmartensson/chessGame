#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include <netdb.h>
#include <netinet/in.h>

//#define SOCK_PATH "/tmp/labnet_socket"
//#define MAX_BYTES 4096
//******** net additions

#define PORT 3490 // the port client will be connecting to
#define MAXDATASIZE 100 // max number of bytes we can get at once

#define WHITE        1
#define BLACK        2

#define wPAWN        10
#define wROOK        11
#define wKNIGHT      12
#define wBISHOP      13
#define wQUEEN       14
#define wKING        15

#define bPAWN        20
#define bROOK        21
#define bKNIGHT      22
#define bBISHOP      23
#define bQUEEN       24
#define bKING        25

int movedPiece=1;
int sendTable=0;
int winner=0;
int connectSocket;
int colour;

int main(int argc, char *argv[])
{
    int x,y,nx,ny,*pawn,*rook,*bishop,*knight,*queen,*king, *currentPlayer=20, *playerVar1, *playerVar2, *playerVar3;
    int chessBoardArray[8][8];
    int errorNum=0;
    char playerName[100];

        //TCP VARS
    char buf[MAXDATASIZE];
    struct hostent *he;
    struct sockaddr_in their_addr; // connector's address information
    struct sockaddr_un remote;



    if (argc != 2) // if more than two commands
    {
        fprintf(stderr,"usage: client hostname\n");
        exit(1);
    }
    if((connectSocket= socket(AF_INET, SOCK_STREAM, 0))==-1)
    {
        perror("socket");
        exit(1);
    }

    if ((he=gethostbyname(argv[1])) == NULL)    // get the host info
    {
        perror("gethostbyname");
        exit(1);
    }
    //strcpy((*he), "130.237.84.198");

// START main Code
// TCP connect code:
    printf("Trying to connect ...\n");

    their_addr.sin_family = AF_INET;    // host byte order
    their_addr.sin_port = htons(PORT);  // short, network byte order
    their_addr.sin_addr = *((struct in_addr *)he->h_addr);
    memset(their_addr.sin_zero, '\0', sizeof their_addr.sin_zero);

    if (connect(connectSocket,(struct sockaddr*)&their_addr,sizeof their_addr)==-1)
    {
        perror("connect");
        exit(1);
    }

    printf("\n");
    printf("Connected.\n");
    printf("\n");
    // Get name and send to server
    printf("Type your username:");
    scanf("%s", &playerName);

     printf("\n PlayerName: ->%s<- \n", playerName);
    if (send(connectSocket, playerName, sizeof(playerName), 0) == -1) {
        perror("send");
        exit(1);
    }
    recv(connectSocket, &colour, sizeof(colour),0);
    printf("colour: %d\n", colour);

    printf("sleeping...\n");
    printf("Waiting for opponent\n");
    recv(connectSocket, chessBoardArray, sizeof(chessBoardArray), 0);

    for(;;){
        if(sendTable==1){
            send(connectSocket, &chessBoardArray, sizeof(chessBoardArray), 0);
            send(connectSocket, &winner, sizeof(winner), 0);
            printBoard(chessBoardArray);
            printf("sleeping...\n");
            printf("Waiting for opponent\n");
            recv(connectSocket, chessBoardArray, sizeof(chessBoardArray), 0);
            sendTable=0;
        }
        checkColour(&currentPlayer, &playerVar1, &playerVar2, &playerVar3, &pawn, &rook, &knight, &bishop, &queen, &king);
        printBoard(chessBoardArray);
        printf("Spelare %d: Vilken pjas vill du flytta? (y x)", currentPlayer);
        scanf("%d %d", &y, &x); /* data för vart pjäsen man vill flytta står */
        if(chessBoardArray[y][x]!=0){


            if(chessBoardArray[y][x]==pawn){tvaBonde(chessBoardArray, x, y, currentPlayer, playerVar1, playerVar2, playerVar3);}
            else if(chessBoardArray[y][x]==rook){tvaTorn(chessBoardArray, x, y, currentPlayer, playerVar1, playerVar2, playerVar3);}
            /*else if(chessBoardArray[y][x]==knight){tvaHast(chessBoardArray, x, y);}
            else if(chessBoardArray[y][x]==bishop){tvaLopare(chessBoardArray, x, y);}
            else if(chessBoardArray[y][x]==queen){tvaDrottning(chessBoardArray, x, y);}
            else if(chessBoardArray[y][x]==king){tvaKung(chessBoardArray, x, y);}      */
            else{
                printf("\nEj rorlig pjas\n");
                sendTable=0;
            }
        }
        else{
            printf("\nTom ruta\n");
            sendTable=0;
        }
    }
    return 0;
}

void tvaBonde(int chessBoardArray[8][8],int x,int y, int currentPlayer, int playerVar1, int playerVar2, int playerVar3, int nx, int ny, int pawnDouble){
        for(;;){
            pawnDouble=playerVar3*2;
            printf("Bonde: Vart vill du flytta pjasen?\n");
            scanf("%d %d", &ny, &nx); /* data för vart pjäses ska flyttas */
            if(ny==y+playerVar3 && nx==x && chessBoardArray[ny][nx]==0){ /* Kollar om rutan framför bonden är tom */
                validMove(chessBoardArray,x,y,nx,ny);
                return;
            }
            else if(ny==y+pawnDouble && chessBoardArray[ny][nx]==0 && (y==6 || y==1)){
                validMove(chessBoardArray,x,y,nx,ny);
                return;
            }
            else if(ny==y+playerVar3 && (nx==x+1 || nx == x-1) && chessBoardArray[ny][nx]>playerVar1 && chessBoardArray[ny][nx]<playerVar2){ /* Kollar ifall användaren kan ta ut en av motståndarens pjäser */
                validMove(chessBoardArray,x,y,nx,ny);
                return;
            }
            else{
                printf("\nInvalid move\n");
            }
        }
}

void tvaTorn(int chessBoardArray[8][8],int x,int y, int currentPlayer, int playerVar1, int playerVar2, int playerVar3, int nx, int ny, int i, int j){
        for(;;){
            printf("Torn:\n");
            printf("Vart vill du flytta pjasen?");
            scanf("%d %d", &ny, &nx);
            if(ny==y || nx==x){
                if(ny==y){ /* Kollar om tornet rörde sig vågrät */
                    if((x-nx)<0){ /* Kollar om den rörde sig åt höger eller vänster (höger i detta fall) */
                        j=1;
                        for(i=nx-x; i>0; i--){ /* kontrollerar i for-loopen om alla rutor i dess väg är tomma */
                            if(chessBoardArray[y][x+j]==0){
                                if(nx-x==1){ /* Om tornet bara tar 1 steg behöver den inte ktrollera om alla rutor är tomma */
                                    validMove(chessBoardArray,x,y,nx,ny);
                                    return;
                                }
                                j++;
                                if(j==nx-x && (chessBoardArray[ny][nx]== 0 ||(chessBoardArray[ny][nx]>playerVar1 && chessBoardArray[ny][nx]<playerVar2))){ /* Om alla rutor den passerar är tomma och destinationen är tom eller har en fiende */
                                    validMove(chessBoardArray,x,y,nx,ny);
                                    return;
                                }
                            }
                            else if(nx-x==1 && (chessBoardArray[ny][nx]== 0 ||(chessBoardArray[ny][nx]>playerVar1 && chessBoardArray[ny][nx]<playerVar2))){ /* Om tornet bara tar 1 steg kollar den om de finns en fiende framför */
                                    validMove(chessBoardArray,x,y,nx,ny);
                                    return;
                            }
                            else{
                                printf("\nInvalid move\n");
                            }
                        }
                    }
                    else if((x-nx)>0){ /* Ifall tornet rör sig åt vänster */
                        j=1;
                        for(i=x-nx; i>0; i--){
                            if(chessBoardArray[y][x-j]==0){
                                if(x-nx==1){
                                    validMove(chessBoardArray,x,y,nx,ny);
                                    return;
                                }
                                j++;
                                if(j==x-nx && (chessBoardArray[ny][nx]== 0 ||(chessBoardArray[ny][nx]>playerVar1 && chessBoardArray[ny][nx]<playerVar2))){
                                    validMove(chessBoardArray,x,y,nx,ny);
                                    return;
                                }
                            }
                            else if(x-nx==1 && (chessBoardArray[ny][nx]== 0 ||(chessBoardArray[ny][nx]>playerVar1 && chessBoardArray[ny][nx]<playerVar2))){
                                    validMove(chessBoardArray,x,y,nx,ny);
                                    return;
                            }
                            else{
                                printf("\nInvalid move\n");
                            }
                        }
                    }
                }
                else if(nx==x){ /* Kollar ifall tornet rör sig lodrät */
                    if((y-ny)<0){ /* Ifall tornet rör sig nedåt */
                        j=1;
                        for(i=ny-y; i>0; i--){
                            if(chessBoardArray[y+j][x]==0){
                                if(ny-y==1){
                                    validMove(chessBoardArray,x,y,nx,ny);
                                    return;
                                }
                                j++;
                                if(j==ny-y && (chessBoardArray[ny][nx]== 0 ||(chessBoardArray[ny][nx]>playerVar1 && chessBoardArray[ny][nx]<playerVar2))){
                                    validMove(chessBoardArray,x,y,nx,ny);
                                    return;
                                }
                            }
                            else if(ny-y==1 && (chessBoardArray[ny][nx]== 0 ||(chessBoardArray[ny][nx]>playerVar1 && chessBoardArray[ny][nx]<playerVar2))){
                                    validMove(chessBoardArray,x,y,nx,ny);
                                    return;
                            }
                            else{
                                printf("\nInvalid move\n");
                            }
                        }
                    }
                    else if((y-ny)>0){ /* Ifall tornet rör sig uppåt */
                        j=1;
                        for(i=y-ny; i>0; i--){
                            if(chessBoardArray[y-j][x]==0){
                                if(y-ny==1){
                                    validMove(chessBoardArray,x,y,nx,ny);
                                    return;
                                }
                                j++;
                                if(j==y-ny && (chessBoardArray[ny][nx]== 0 ||(chessBoardArray[ny][nx]>playerVar1 && chessBoardArray[ny][nx]<playerVar2))){
                                    validMove(chessBoardArray,x,y,nx,ny);
                                    return;
                                }
                            }
                            else if(y-ny==1 && (chessBoardArray[ny][nx]== 0 ||(chessBoardArray[ny][nx]>playerVar1 && chessBoardArray[ny][nx]<playerVar2))){
                                    validMove(chessBoardArray,x,y,nx,ny);
                                    return;
                            }
                            else{
                                printf("\nInvalid move\n");
                            }
                        }
                    }
                }
            }
        }
}

void printBoard(int chessBoardArray[8][8],int j,int i){
    for(i=0; i<8; i++){
            for(j=0; j<8; j++)
            {
                printf("\t %d",chessBoardArray[i][j]);
            }
            printf("\n\n");
    }
    return;
}

void checkColour(int *currentPlayer, int *playerVar1, int *playerVar2, int *playerVar3, int *pawn, int *rook, int *knight, int *bishop, int *queen, int *king){
    if(colour==WHITE){
        *currentPlayer=10;
        *playerVar1=19;
        *playerVar2=26;
        *playerVar3=1;
        *pawn=10;
        *rook=11;
        *knight=12;
        *bishop=13;
        *queen=14;
        *king=15;
    }
    else if(colour==BLACK){
        *currentPlayer=20;
        *playerVar1=9;
        *playerVar2=16;
        *playerVar3=-1;
        *pawn=20;
        *rook=21;
        *knight=22;
        *bishop=23;
        *queen=24;
        *king=25;
    }
    movedPiece=0;
}

void validMove(int chessBoardArray[8][8],int x,int y, int nx, int ny){
    chessBoardArray[ny][nx]=chessBoardArray[y][x];
    chessBoardArray[y][x]=0;
    sendTable=1;
    movedPiece=1;
    return;
}
