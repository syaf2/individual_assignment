#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <signal.h>

#define QTY_LENGTH 4
#define QTY_WAIT_INTERVAL 2

void generateRandomQuantity(char qty[QTY_LENGTH + 1]) {
    srand(getpid() + getppid());
    qty[0] = 49 + rand() % 7;
    for (int i = 1; i < QTY_LENGTH; i++) {
        qty[i] = 48 + rand() % 7;
    }
    qty[QTY_LENGTH] = '\0';
}

void handleSigint(int sig) {
    printf("**** Process interrupted by user! ****\n");
    exit(1);
}

int main() {
    if (signal(SIGINT, handleSigint) == SIG_ERR) {
        perror("Error setting up signal handler");
        exit(EXIT_FAILURE);
    }

    while (1) {
        int pipefds[2];
        char qty[QTY_LENGTH + 1];
        char buffer[QTY_LENGTH + 1];
        char input[100];

        if (pipe(pipefds) == -1) {
            perror("Pipe creation failed");
            exit(EXIT_FAILURE);
        }

        pid_t pid = fork();

        if (pid == -1) {
            perror("Fork failed");
            exit(EXIT_FAILURE);
        }

        if (pid == 0) {
            // Child process
            if (signal(SIGINT, SIG_IGN) == SIG_ERR) {
                perror("Error ignoring SIGINT in child");
                exit(EXIT_FAILURE);
            }

            generateRandomQuantity(qty);
            close(pipefds[0]);
            write(pipefds[1], qty, QTY_LENGTH + 1);

            printf("Please enter your favorite color: \n");
            fgets(input, sizeof(input), stdin);
            printf("Your favorite color is: %s", input);
            printf("Parent generating quantity and sending to child...\n");

            sleep(QTY_WAIT_INTERVAL);

            exit(EXIT_SUCCESS);
        } else {
            // Parent process
            wait(NULL);
            close(pipefds[1]);
            read(pipefds[0], buffer, QTY_LENGTH + 1);
            printf("Child received quantity '%s' from parent...\n\n", buffer);
        }
    }

    return EXIT_SUCCESS;
}
