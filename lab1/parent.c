#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>

int main(void) {
    const char p[] = "Enter filename: ";
    if (write(STDOUT_FILENO, p, sizeof(p) - 1) == -1)
        return EXIT_FAILURE;

    char filename_of_title[1024], c;
    int length_title_bytes = 0;
    ssize_t bytes;

    while ((bytes = read(STDIN_FILENO, &c, 1)) == 1 && c != '\n') {
        if (length_title_bytes == 1023 || c == '\0'){
        return EXIT_FAILURE;
    }
        filename_of_title[length_title_bytes] = c;
        length_title_bytes++;
    }
    if (bytes == -1 || length_title_bytes == 0) {
        return EXIT_FAILURE;
    }
    filename_of_title[length_title_bytes] = '\0';

    int parent_to_child[2], child_to_parent[2];
    if (pipe(parent_to_child) == -1){
        return EXIT_FAILURE;
    }
    if (pipe(child_to_parent) == -1){
        return EXIT_FAILURE;
    }
    if (signal(SIGPIPE, SIG_IGN) == SIG_ERR){
        return EXIT_FAILURE;
    }
    pid_t child = fork();
    if (child == -1){
        return EXIT_FAILURE;
    }

    if (child == 0) {
        close(parent_to_child[1]);
        close(child_to_parent[0]);

        if (dup2(parent_to_child[0], STDIN_FILENO) == -1){
        return EXIT_FAILURE;
    }
        if (dup2(child_to_parent[1], STDOUT_FILENO) == -1){
        return EXIT_FAILURE;
    }

        close(parent_to_child[0]);
        close(child_to_parent[1]);
        char *args[] = {"./child", filename_of_title, NULL};
        execv("./child", args); 
        return EXIT_FAILURE;    
    } else {
        close(parent_to_child[0]);
        close(child_to_parent[1]);

        char ready;
        int success = read(child_to_parent[0], &ready, 1) == 1;
        int unfinished = 0;

        while (success) {
            bytes = read(STDIN_FILENO, &c, 1);
            if (bytes == -1) {
                success = 0;
                break;
            }
            if (bytes == 0) {
                if (!unfinished)
                    break;
                c = '\n';
            } else if (c == '\n' && !unfinished) {
                break;
            }
            if (write(parent_to_child[1], &c, 1) != 1) {
                success = 0;
                break;
            }
            unfinished = c != '\n';
            if (c == '\n')
                success = read(child_to_parent[0], &ready, 1) == 1;
            if (bytes == 0)
                break;
        }

        close(parent_to_child[1]);
        close(child_to_parent[0]);

        int status;
        if (waitpid(child, &status, 0) == -1 ||
            !success || !WIFEXITED(status)){
        return EXIT_FAILURE;
    }

        return WEXITSTATUS(status);
    }
}