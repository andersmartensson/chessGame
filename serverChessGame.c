#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>


#define SOCK_PATH "/tmp/labnet_socket"
#define FILE_PATH "/tmp/lab_serverpid"
#define MAXCLIENTS 10
#define NAMELENGTH 100

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

//#define MAIN LOG_FILE "/tmp/mainserver.log" // USED FOR LOGGING
#define PATH "/tmp/"


// TCP Defines
#define MYPORT 3490 // the port users will be connecting to
#define BACKLOG 10 // how many pending connections queue will hold

/*struct
{


};*/


int main(int argc, char *argv[])
{
    int connectSock, clientSock, clientSock1, clientSock2; //len, acceptAdressLength
    int pid;
    int date; //To be con....
    char playerOne[100], playerTwo[100];
    struct sockaddr_in gameServerAddress; //my address info
    struct sockaddr_in gameClientAddress;
    socklen_t sin_size;
    int winner=0;
    int sizeofClientAddress;
    struct sockaddr_un local, remote;   //For connection
    int yes = 1; // TCP connection variable
    int WHITE=1, BLACK=2;

    //FILE *mainlog = fopen("/tmp/serverlog","w");
    //Write header for file.
    //fprintf("date: %d, Player 1: %s, Player 2: %s", date, playerOne, playerTwo);

int chessBoardArray[8][8] = {
        {wROOK,wKNIGHT,wBISHOP,wQUEEN,wKING,wBISHOP,wKNIGHT,wROOK}, /* spelare 1: torn=11 häst=12 löpare=13 drottning=14 kung=15 */
        {wPAWN,wPAWN,wPAWN,wPAWN,wPAWN,wPAWN,wPAWN,wPAWN}, /* bonde=10 */

        {0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0},

        {bPAWN,bPAWN,bPAWN,bPAWN,bPAWN,bPAWN,bPAWN,bPAWN}, /* bonde=20 */
        {bROOK,bKNIGHT,bBISHOP,bQUEEN,bKING,bBISHOP,bKNIGHT,bROOK}  /* spelare 2: torn=21 häst=22 löpare=23 drottning=24 kung=25 */
};
    //Start connection
    if((connectSock = socket(AF_INET, SOCK_STREAM,0)) == -1)
    {
        perror("Socket: ");
        exit(1);
    }
    //Connection done

    if(setsockopt(connectSock, SOL_SOCKET, SO_REUSEADDR, &yes,sizeof(int)) == -1)
    {
        perror("setsockopt");
        exit(1);
    }

    gameServerAddress.sin_family = AF_INET;   //Host byte order.
    gameServerAddress.sin_port = htons(MYPORT); //Short, network byte order
    gameServerAddress.sin_addr.s_addr = INADDR_ANY;   //Automatically filled with my IP
    memset(gameServerAddress.sin_zero, '\0', sizeof gameServerAddress.sin_zero);

    if(bind(connectSock,(struct sockaddr *)&gameServerAddress, sizeof gameServerAddress) == -1)
    {
        perror("bind");
        printf("Bind successful. local-sun_path: %s Exiting\n",local.sun_path);
        exit(1);

    }
    if(listen(connectSock, BACKLOG) == -1)
    {
        perror("listen");
        exit(1);
    }

    //Main-loop
    for(;;) //Etarnal loop
    {
        printf("Server<%d> Waiting for connection...\n",getpid());
        sizeof gameClientAddress;   //Why here'
        sin_size = sizeof gameClientAddress;

        if((clientSock1 = accept(connectSock, (struct sockaddr *)&gameClientAddress, &sin_size)) == -1)
        {
            perror("accept");
            continue;
        }
        //Receives name from player 1
        recv(clientSock1, playerOne, sizeof(playerOne),0);
        send(clientSock1, &WHITE, sizeof(WHITE), 0);
        printf("Player 1:  >%s<  \n", playerOne);
        printf("Colour: %d\n", WHITE);

        if((clientSock2 = accept(connectSock, (struct sockaddr *)&gameClientAddress, &sin_size)) == -1)
        {
            perror("accept");
            continue;
        }
        recv(clientSock2, playerTwo, sizeof(playerTwo),0);
        send(clientSock2, &BLACK, sizeof(BLACK), 0);
        printf("Player 2: >%s< \n",playerTwo);
        printf("Colour: %d\n", BLACK);

        //FILE *mainlog = fopen("/tmp/","w");
        //fprintf("date: %d, Player 1: %s, Player 2: %s", date, playerOne, playerTwo);
        printf("%d server: got connection from\n", getpid());


        pid = fork(); // F O R K    H E R E  ! ! ! ! ! ! !
        if(pid!=0)
        {
            printf("DEBUG: Server<%d>  closing Sockets\n", getpid());
//            close(clientSock1);
//            close(clientSock2);
            //fclose(mainlog);

        }
        if(pid==0) // C H I L D     P R O C C E S S   O N L Y
        {
            close(connectSock);
            printf("c: %d Server says: Connected.\n", getpid());
            //Main-loop
            for(;;)
            {

                printf("DEBUG: Server<%d>  player %s turn\n", getpid(), playerOne);

                printf("DEBUG: Server<%d>  player  %s  send board\n", getpid(), playerOne);
                send(clientSock1, &chessBoardArray, sizeof(chessBoardArray), 0);

                printf("DEBUG: Server<%d>  player  %s  rec board\n", getpid(), playerOne);
                recv(clientSock1, &chessBoardArray, sizeof(chessBoardArray), 0);

                printf("DEBUG: Server<%d>  player  %s  rec winner\n", getpid(), playerOne);
                recv(clientSock1, &winner, sizeof(winner), 0);

                if(winner == 1)
                {
                    printf("%s won!\n",playerOne);
                    //fprintf(mainlog, "%s");
                    //fclose(mainlog);
                }


                printf("DEBUG: Server<%d>  player %s turn\n", getpid(), playerTwo);
                send(clientSock2, &chessBoardArray, sizeof(chessBoardArray),0);

                printf("DEBUG: Server<%d>  player  %s  rec board\n", getpid(), playerTwo);
                recv(clientSock2, &chessBoardArray, sizeof(chessBoardArray),0);

                printf("DEBUG: Server<%d>  player %s rec winner\n", getpid(), playerTwo);
                recv(clientSock2, &winner, sizeof(winner),0);
                if(winner == 1)
                {
                    printf("%s won!",playerTwo);
                    //fprintf(mainlog, "%s");
                    //fclose(mainlog);
                }
            }

        }

    }
    return 0;
}
