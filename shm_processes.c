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
    sem_unlink("/bank_mutex");

    return 0;
}

//Parent Process: Dear Old Dad
void DearOldDad(int *BankAccount, sem_t *mutex) {
    srandom(time(NULL) ^ getpid()); // Seed random number generator

    while (1) {
        sleep(random() % 6); // Sleep 0 - 5 seconds
        printf("Dear Old Dad: Attempting to Check Balance\n");

        sem_wait(mutex); // Enter Critical Section
        int localBalance = *BankAccount;
        int randomNumber = random();

        if (randomNumber % 2 == 0) { // Even number: Deposit decision
            if (localBalance < 100) {
                int amount = random() % 101; // Deposit 0 - 100
                if (amount % 2 == 0) {
                    localBalance += amount;
                    *BankAccount = localBalance;
                    printf("Dear old Dad: Deposits $%d / Balance = $%d\n", amount, localBalance);
                } else {
                    printf("Dear old Dad: Doesn't have any money to give\n");
                }
            } else {
                printf("Dear old Dad: Thinks Student has enough Cash ($%d)\n", localBalance);
            }
        } else { // Odd number: Just check balance
            printf("Dear Old Dad: Last Checking Balance = $%d\n", localBalance);
        }

        sem_post(mutex); // Exit Critical Section
    }
}


//Child Process: Poor Student
void  PoorStudent(int *BankAccount, sem_t *mutex){
    srandom(time(NULL) ^ getpid()); // Seed random generator uniquely per process

    while (1) {
        sleep(random() % 6); // Sleep 0 - 5 seconds
        printf("Poor Student: Attempting to Check Balance\n");

        sem_wait(mutex); // Enter Critical Section
        int localBalance = *BankAccount;
        int randomNumber = random();

        if (randomNumber % 2 == 0) { // Even number: Attempt Withdraw
            int need = random() % 51; // Need 0 - 50
            printf("Poor Student needs $%d\n", need);

            if (need <= localBalance) {
                localBalance -= need;
                *BankAccount = localBalance;
                printf("Poor Student: Withdraws $%d / Balance = $%d\n", need, localBalance);
            } else {
                printf("Poor Student: Not Enough Cash ($%d)\n", localBalance);
            }
        } else { // Odd number: Just check balance
            printf("Poor Student: Last Checking Balance = $%d\n", localBalance);
        }

        sem_post(mutex); // Exit Critical Section
    }
}