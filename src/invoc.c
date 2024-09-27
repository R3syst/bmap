#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#define CMD_BUFFER_SIZE 256
#define OUTPUT_BUFFER_SIZE 4096

void execute_gdb_command(const char* binary_path, const char* gdb_command) {
    int pipes[2];
    if (pipe(pipes) == -1) {
        perror("pipe failed");
        exit(1);
    }

    pid_t pid = fork();
    if (pid == -1) {
        perror("fork failed");
        exit(1);
    }

    if (pid == 0) {
        close(pipes[0]); 
        dup2(pipes[1], STDOUT_FILENO);
        close(pipes[1]); 

        execlp("gdb", "gdb", "-q", "-ex", gdb_command, "-ex", "quit", binary_path, (char *)NULL);

        // If execlp fails
        perror("execlp failed");
        exit(1);
    } else {
        close(pipes[1]); 

        char buffer[OUTPUT_BUFFER_SIZE];
        ssize_t count = read(pipes[0], buffer, OUTPUT_BUFFER_SIZE - 1);
        if (count > 0) {
            buffer[count] = '\0'; // Null terminate the output
            printf("%s\n", buffer); // Print GDB output (or process it further)
        }

        close(pipes[0]); 
        wait(NULL); 
    }
}

void analyze_binary(const char *binary_path) {
    printf("Analyzing binary: %s\n", binary_path);
    execute_gdb_command(binary_path, "info functions");
    execute_gdb_command(binary_path, "info breakpoints");
    execute_gdb_command(binary_path, "disassemble /m main");
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <binary_file>\n", argv[0]);
        return 1;
    }

    const char *binary_path = argv[1];

    analyze_binary(binary_path);

    return 0;
}
