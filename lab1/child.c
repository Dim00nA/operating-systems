#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <limits.h>

void write_int(int file, int value) {
    char buf[32], temp[32];
    int length = 0, count = 0;
    long long number = value;
    if (number < 0) {
        buf[length++] = '-';
        number = -number;
    }
    do {
        temp[count++] = (char)('0' + number % 10);
        number /= 10;
    } while (number > 0);
    while (count > 0)
        buf[length++] = temp[--count];
    buf[length++] = '\n';
    if (write(file, buf, length) != length) {
        const char msg[] = "error: failed to write result\n";
        write(STDERR_FILENO, msg, sizeof(msg) - 1);
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char **argv) {
    if (argc != 2)
        return EXIT_FAILURE;
    int file = open(argv[1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (file == -1) {
        const char msg[] = "error: failed to open file\n";
        write(STDERR_FILENO, msg, sizeof(msg) - 1);
        return EXIT_FAILURE;
    }
    if (write(STDOUT_FILENO, "\n", 1) != 1)
        return EXIT_FAILURE;

    char token[64], c;
    int length = 0, result = 0, first = 1;
    ssize_t bytes;
    while ((bytes = read(STDIN_FILENO, &c, 1)) == 1) {
        int separator = strchr(" \t\r\n\v\f", c) != NULL;
        if (c == '\0' || (length == 63 && !separator)) {
            const char msg[] = "error: invalid integer\n";
            write(STDERR_FILENO, msg, sizeof(msg) - 1);
            return EXIT_FAILURE;
        }
        if (!separator) {
            token[length++] = c;
            continue;
        }
        if (length > 0) {
            token[length] = '\0';
            char *end;
            long long value = strtoll(token, &end, 10);
            if (end == token || *end != '\0' ||
                value < INT_MIN || value > INT_MAX) {
                const char msg[] = "error: invalid integer\n";
                write(STDERR_FILENO, msg, sizeof(msg) - 1);
                return EXIT_FAILURE;
            }
            if (first) {
                result = (int)value;
                first = 0;
            } else if (value == 0) {
                const char msg[] = "error: division by zero\n";
                write(STDERR_FILENO, msg, sizeof(msg) - 1);
                return EXIT_FAILURE;
            } else if (result == INT_MIN && value == -1) {
                const char msg[] = "error: integer overflow\n";
                write(STDERR_FILENO, msg, sizeof(msg) - 1);
                return EXIT_FAILURE;
            } else {
                result /= (int)value;
            }
            length = 0;
        }
        if (c == '\n') {
            if (!first)
                write_int(file, result);
            if (write(STDOUT_FILENO, "\n", 1) != 1)
                return EXIT_FAILURE;
            first = 1;
        }
    }
    if (bytes < 0) {
        const char msg[] = "error: failed to read input\n";
        write(STDERR_FILENO, msg, sizeof(msg) - 1);
        return EXIT_FAILURE;
    }
    if (close(file) == -1)
        return EXIT_FAILURE;
    return EXIT_SUCCESS;
}
