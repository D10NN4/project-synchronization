#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <semaphore.h>
#include <fcntl.h>
#include <time.h>

void  DearOldDad(int *BankAccount, sem_t *mutex);
void  PoorStudent(int *BankAccount, sem_t *mutex);

int  main(int  argc, char *argv[]){
     int ShmID;
     int *BankAccount;
     sem_t *mutex;

     //1. Create Shared Memory for BankAccount
     ShmID = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | 0666);
     if (ShmID < 0) {
          perror("*** shmget error ***");
          exit(1);
     }

     BankAccount = (int *) shmat(ShmID, NULL, 0);
     if (*BankAccount == -1) {
          perror("*** shmat error ***");
          exit(1);
     }

    //Initialize BankAccount to 0
    *BankAccount = 0;
     
    //2. Initialized Named semaphore
    sem_unlink("/bank_mutex");
    mutex = sem_open("/bank_mutex", 0_CREAT, 0644, 1);
    if (mutex == SEM_FAILED){
      perror("*** sem_open error ***")
      exit(1);
    }

    //3. Fork Child Process
    pid_t pid = fork();
    if (pid < 0) {
        perror("*** fork error ***");
        exit(1);
    } else if (pid == 0) {
        PoorStudent(BankAccount, mutex);
        exit(0);
    } else {
        //Parent Process
        DearOldDad(BankAccount, mutex);
        wait(NULL);
    }

    //Cleanup
    shmdt((void *) BankAccount);
    shmctl(ShmID, IPC_RMID, NULL);
    sem_close(mutex);
    sem_unlink("/bank_mutex")
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