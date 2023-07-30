#ifdef _WIN32
#include <io.h>
#else
#ifdef __linux__
#include <unistd.h>
#else
#error OS not supported.
#endif
#endif

#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bool file_exist(const char *filename) {
#ifdef _WIN32
    if (_access(filename, 0) != -1) {
        return true;
    }
#endif
#ifdef __linux__
    if (access(filename, F_OK) == 0) {
        return true;
    }
#endif
    return false;
}

void usage(char *msg) {
    printf("Usage: %s <file name> <header guard> <path (optional)>\n", msg);
    printf("For more informations: %s help\n", msg);
}

void help(char *msg) {
    printf("************************************\n");
    usage(msg);
    printf("\n");
    printf("Example : %s add MATHS_ADD /calc\n", msg);
    printf("Example : %s add MATHS_ADD /calc/\n", msg);
    printf("Will generate 2 files : add.c and add.h with the\n");
    printf("header guard \"MATHS_ADD_H\", inside a directory\n");
    printf("called \"calc\" relative to the current working directory.\n");
    printf("************************************\n");
}

int main(int argc, char **argv) {
    if (argc == 2) {
        if (memcmp(argv[1], "help", 4 * sizeof(char)) == 0) {
            help(argv[0]);
            return EXIT_SUCCESS;
        } else {
            printf("%s command not recognised.\n", argv[0]);
            printf("Type: %s help\n", argv[0]);
            return EXIT_FAILURE;
        }
    }
    if ((argc != 3) && (argc != 4)) {
        usage(argv[0]);
        return EXIT_FAILURE;
    }

    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        perror("Could not get current working directory path");
        return EXIT_FAILURE;
    }
    unsigned int cwdlen = strlen(cwd);

    const char  *filename = argv[1];
    unsigned int filenamelen = strlen(filename);
    char        *header_guard = argv[2];
    unsigned int header_guardlen = strlen(header_guard);
    char        *path = "/";
    unsigned int pathlen = 1;

    if (argc == 4) {
        path = argv[3];
        pathlen = strlen(path);
        if (path[pathlen - 1] != '/') {
            path[pathlen] = '/';
            ++pathlen;
        }
    }

    // cwd + path + filename + .c\0
    unsigned int buffer_size = cwdlen + pathlen + filenamelen + 3;
    char        *buffer = malloc(buffer_size * sizeof(char));
    if (buffer == NULL) {
        perror("Could not allocate buffer space");
        return EXIT_FAILURE;
    }
    memset(buffer, '\0', buffer_size * sizeof(char));

    memcpy(buffer, cwd, cwdlen * sizeof(char));
    memcpy(buffer + cwdlen, path, pathlen * sizeof(char));
    memcpy(buffer + cwdlen + pathlen, filename, filenamelen * sizeof(char));
    buffer[buffer_size - 3] = '.';

    buffer[buffer_size - 2] = 'h';
    if (file_exist(buffer)) {
        printf("WARNING: %s already exist, canceling now.\n", buffer);
        return EXIT_SUCCESS;
    }
    {
        FILE *header = fopen(buffer, "wa");
        if (header == NULL) {
            printf("ERROR: Can't create %s\n", buffer);
            return EXIT_FAILURE;
        }
        const char *ifndef_begin = "#ifndef ";
        const char *def_begin = "#define ";
        const char *cpp = "\n#ifdef __cplusplus\nextern \"C\" "
                          "{\n#endif\n\n\n\n#ifdef __cplusplus\n}\n#endif\n\n";
        const char *last_endif = "#endif // ";
        const char *guard_end = "_H\n";

        fwrite(ifndef_begin, sizeof(char), 8, header);
        fwrite(header_guard, sizeof(char), header_guardlen, header);
        fwrite(guard_end, sizeof(char), 3, header);

        fwrite(def_begin, sizeof(char), 8, header);
        fwrite(header_guard, sizeof(char), header_guardlen, header);
        fwrite(guard_end, sizeof(char), 3, header);

        fwrite(cpp, sizeof(char), 72, header);
        fwrite(last_endif, sizeof(char), 10, header);
        fwrite(header_guard, sizeof(char), header_guardlen, header);
        fwrite(guard_end, sizeof(char), 3, header);

        fclose(header);
        printf("SUCCESS: %s created.\n", buffer);
    }

    buffer[buffer_size - 2] = 'c';
    if (file_exist(buffer)) {
        printf("WARNING: %s already exist, canceling now.\n", buffer);
        return EXIT_SUCCESS;
    }
    {
        FILE *src = fopen(buffer, "wa");
        if (src == NULL) {
            printf("ERROR: Can't create %s\n", buffer);
            return EXIT_FAILURE;
        }
        const char *include_begin = "#include \"";
        const char *include_end = ".h\"\n";

        fwrite(include_begin, sizeof(char), 10, src);
        fwrite(filename, sizeof(char), filenamelen, src);
        fwrite(include_end, sizeof(char), 4, src);

        fclose(src);
        printf("SUCCESS: %s created.\n", buffer);
    }
    printf("DONE\n");
    return EXIT_SUCCESS;
}
