#include <cstdio>
#include <cstdlib>
#include <string>
#include <unistd.h>

#include "Debug.h"
#include "Util.h"
#include "FileUtil.h"
#include "SubstitutionMatrix.h"

const char *binary_name = "test_lambdacalculator";

int main(int argc, const char ** argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <matrix_file>\n", binary_name);
        return EXIT_FAILURE;
    }

    FILE* infile = FileUtil::openFileOrDie(argv[1], "r", true);

    std::string filtered_content;
    char* line = nullptr;
    size_t len = 0;
    ssize_t read;
    while ((read = getline(&line, &len, infile)) != -1) {
        if (read > 0 && line[0] == '#') continue;
        if (read > 0 && line[read - 1] == '\n') {
            line[read - 1] = '\0';
        }
        filtered_content += line;
        filtered_content += '\n';
    }
    free(line);
    fclose(infile);

    char tmpname[] = "/tmp/matrixXXXXXX.out";
    int fd = mkstemp(tmpname);
    if (fd == -1) {
        fprintf(stderr, "Failed to create temporary file\n");
        return EXIT_FAILURE;
    }
    FILE* tmpfile = fdopen(fd, "w+");
    if (!tmpfile) {
        fprintf(stderr, "Failed to open temporary file stream\n");
        close(fd);
        return EXIT_FAILURE;
    }

    if (!filtered_content.empty()) {
        fwrite(filtered_content.c_str(), 1, filtered_content.size(), tmpfile);
    }
    fflush(tmpfile);

    SubstitutionMatrix mat(tmpname, 2.0f, 0.0f);

    if (!filtered_content.empty()) {
        fwrite(filtered_content.c_str(), 1, filtered_content.size(), stdout);
    }

    fclose(tmpfile);
    unlink(tmpname);

    return EXIT_SUCCESS;
}
