/*
 * Word count application with one process per input file.
 *
 * You may modify this file in any way you like, and are expected to modify it.
 * Your solution must read each input file from a separate thread. We encourage
 * you to make as few changes as necessary.
 */

/*
 * Copyright © 2019 University of California, Berkeley
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <ctype.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "word_count.h"
#include "word_helpers.h"

/*
 * Read stream of counts and accumulate globally.
 */

void merge_counts(word_count_list_t *wclist, FILE *count_stream) {
    char *word;
    int count;
    int rv;
    while ((rv = fscanf(count_stream, "%8d\t%ms\n", &count, &word)) == 2) {
        add_word_with_count(wclist, word, count);
    }
    if ((rv == EOF) && (feof(count_stream) == 0)) {
        perror("could not read counts");
    } else if (rv != EOF) {
        fprintf(stderr, "read ill-formed count (matched %d)\n", rv);
    }
}

/*
 * main - handle command line, spawning one process per file.
 */
int main(int argc, char *argv[]) {
    /* Create the empty data structure. */
    word_count_list_t word_counts;
    init_words(&word_counts);
    int bound = argc - 1; //1 = self
    int pipefd[2];

    

    if (argc <= 1) {
        /* Process stdin in a single process. */
        count_words(&word_counts, stdin);
    } else {
        //pipe, parent writes file name

        int pid;

        for (int i = 0; i < bound; i++){

            if (pipe(pipefd) == -1) {
                perror("pipe failed");
                exit(1);
            }

            pid = fork(); //no error, error squiggles are being dramatic today

           
            if (pid == 0) { // child process
                close(pipefd[0]); //not using it
                FILE *file = fopen(argv[i + 1], "r");
                word_count_list_t locallist;
                init_words(&locallist);
                count_words(&locallist, file);
                FILE *printStream = fdopen(pipefd[1], "w"); //child writes result from fprint to this one
                fprint_words(&locallist, printStream);

                fclose(printStream);
                fclose(file);

                exit(0);
            } else { // parent process
                close(pipefd[1]); // not using it
                FILE *mergeReadStream = fdopen(pipefd[0], "r"); //parent reads what child wrote
                merge_counts(&word_counts, mergeReadStream);

                fclose(mergeReadStream);  
            }

        }
        
    }

    /* Output final result of all process' work. */
    wordcount_sort(&word_counts, less_count);
    fprint_words(&word_counts, stdout);
    return 0;
}
