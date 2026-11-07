#include  <stdio.h>
#include  <stdlib.h>
#include  <sys/types.h>
#include  <sys/ipc.h>
#include  <sys/shm.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>

void  DearOldDad(int ShmPTR[]);
void  PoorStudent(int ShmPTR[]);

int  main(int  argc, char *argv[]){
     int    ShmID;
     int    *ShmPTR;
     pid_t  pid;
     int    status;

     //Random number generator
     srand(time(NULL));


     ShmID = shmget(IPC_PRIVATE, 2 * sizeof(int), IPC_CREAT | 0666);
     if (ShmID < 0) {
          printf("*** shmget error (server) ***\n");
          exit(1);
     }
     printf("Server has received a shared memory of two integers...\n");

     ShmPTR = (int *) shmat(ShmID, NULL, 0);
     if (*ShmPTR == (int *) -1) {
          printf("*** shmat error (server) ***\n");
          exit(1);
     }
     printf("Server has attached the shared memory...\n");

     ShmPTR[0] = 0;
     ShmPTR[1] = 0;
     
     printf("Server has filled %d %d in shared memory...\n",
            ShmPTR[0], ShmPTR[1]);

     printf("Server is about to fork a child process...\n");

     pid = fork();
     if (pid < 0) {
          printf("*** fork error (server) ***\n");
          exit(1);
     }
     else if (pid == 0) {
          PoorStudent(ShmPTR);
          exit(0);
     }
     else {
      DearOldDad(ShmPTR);
      wait(&status);
     
      printf("Server has detected the completion of its child...\n");
      shmdt((void *) ShmPTR);
      printf("Server has detached its shared memory...\n");
      shmctl(ShmID, IPC_RMID, NULL);
      printf("Server has removed its shared memory...\n");
      printf("Server exits...\n");
      exit(0);
     }
}

void  DearOldDad(int  ShmPTR[]){
    int account;
    int balance;

    for (int i = 0; i < 25; i++) {
     //sleep 0 to 5 sec
     sleep(rand() % 6);

     //wait for turn
     while (ShmPTR[1] != 0);

     account = ShmPTR[0];

     //Check account balance
     if ( account <= 100){
        balance = rand() % 101;
        if (balance % 2 == 0){
         account += balance;
         printf("Dear old Dad: Deposits $%d / Balance = $%d\n", balance, account);
        } else {
         printf("Dear old Dad: Doesn't have any money to give\n");
        }
     } else {
      printf("Dear old Dad: Thinks Student has enough Cash ($%d)\n", account);
     }

     //copy account to shared mem, yield turn to child
     ShmPTR[0] = account;
     ShmPTR[1] = 1;
    }
}

void  PoorStudent(int  ShmPTR[]){
    int account;
    int balance;

    for (int i = 0; i < 25; i++) {
     //sleep 0 to 5 sec
     sleep(rand() % 6);

     //wait for turn
     while (ShmPTR[1] != 1);

     //copy bank account to local variable
     account = ShmPTR[0];

     //Generate amount needed
     balance = rand() % 51;
     printf("Poor Student needs $%d\n", balance);

     if (balance <= account){
         account -= balance;
         printf("Poor Student: Withdraws $%d / Balance = $%d\n", balance, account);
        } else {
         printf("Poor Student: Not Enough Cash ($%d)\n", account);
        }

     //copy account to shared mem, yield turn to child
     ShmPTR[0] = account;
     ShmPTR[1] = 0;
    }
}