/*
 * Word count application with one thread per input file.
 *
 * You may modify this file in any way you like, and are expected to modify it.
 * Your solution must read each input file from a separate thread. We encourage
 * you to make as few changes as necessary.
 */

/*
 * Copyright (C) 2019 University of California, Berkeley
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
#include <pthread.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "word_count.h"
#include "word_helpers.h"

#define NUM_THREADS 4
int common = 162;
char *somethingshared;

typedef struct {
    char* filename;
    pthread_mutex_t *localLock;
    word_count_list_t *sharedWordCount;
} threadData;

/*
 * main - handle command line, spawning one thread per file.
 */

void *threadInit(void* argument) {
    
    threadData *data = (threadData*)argument;

    //printf("Opening File: %s\n", data->filename);
  //  int myid = pthread_self();
   // printf("Thread ID: %d\n", myid);
   // printf("Thread for %s started\n", data->filename);
   // sleep(3); 
/*
    if (!data || !data->filename) {
       // fprintf(stderr, "Invalid thread data or filename\n");
        pthread_exit(NULL);
    } */

    FILE *file = fopen(data->filename, "r");
    /*
    if (!file) {
        perror("File open error");
        pthread_exit(NULL);  
    }
*/
    word_count_list_t locallist;
    init_words(&locallist);
    count_words(&locallist, file);
    fclose(file);

    pthread_mutex_lock(data->localLock);
    //critical section, adding local data to shared list
   // printf("In critical section, Thread ID: %d\n", myid);
    struct list *ptr = &locallist.lst;
    struct list_elem *wc = list_begin(ptr);
    for (wc = list_begin(ptr); wc != list_end(ptr); wc = list_next(wc)){
        word_count_t *element = list_entry(wc, word_count_t, elem);
        word_count_t *newElement = add_word(data->sharedWordCount, element->word);
        int count = element->count; 
        for (int i = 1; i < count; i++ ){
            newElement->count = newElement->count + 1;
        }
    }
    pthread_mutex_unlock(data->localLock);
    return NULL;

}





int main(int argc, char *argv[]) {
    /* Create the empty data structure. */
    word_count_list_t word_counts;
    init_words(&word_counts);

    int nthreads = argc - 1;
    threadData tArgs[nthreads];
    pthread_t threads[nthreads];
    pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
    

    if (argc <= 1) {
        count_words(&word_counts, stdin);
    } else {
        for (int i = 0; i < nthreads; i++){
            tArgs[i].filename = argv[i + 1];
            tArgs[i].sharedWordCount = &word_counts;
            tArgs[i].localLock = &lock;
            pthread_create(&threads[i], NULL, threadInit, &tArgs[i]);

            //printf(tArgs[i].filename);
        }
    }

    // Output final result of all threads' work. 

    for (int i = 0; i < nthreads; i++) {
        pthread_join(threads[i], NULL);
    }

    wordcount_sort(&word_counts, less_count);
    fprint_words(&word_counts, stdout);

    pthread_exit(NULL);

    return 0;

}
