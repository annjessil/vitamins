/*
 * Implementation of the word_count interface using Pintos lists and pthreads.
 *
 * You may modify this file, and are expected to modify it.
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

#ifndef PINTOS_LIST
//#error "PINTOS_LIST must be #define'd when compiling word_count_lp.c"
#endif

#ifndef PTHREADS
//#error "PTHREADS must be #define'd when compiling word_count_lp.c"
#endif

#include "word_count.h"

void init_words(word_count_list_t *wclist) {
    
    list_init(&wclist->lst); 
}

size_t len_words(word_count_list_t *wclist) {
    return list_size(&wclist->lst);
}

word_count_t *find_word(word_count_list_t *wclist, char *word) {
    struct list_elem *wc = list_begin(&wclist->lst);
    for (wc = list_begin(&wclist->lst); wc != list_end(&wclist->lst); wc = list_next(wc)){
        word_count_t *element = list_entry(wc, word_count_t, elem);
        if (strcmp(word, element->word) == 0){
            return element;
        }
    }

    return NULL;
}

word_count_t *add_word(word_count_list_t *wclist, char *word) {
    word_count_t *wc = find_word(wclist, word);
    if (wc != NULL){
        wc->count = wc->count + 1;
        return wc;
    } else {
        word_count_t *newWC = malloc(sizeof(word_count_t));
        if (newWC == NULL){
            return NULL; //in case malloc doesnt work
        }

        newWC->count = 1;
        newWC->word = word; //strdup(word), if having issues
        pthread_mutex_lock(&wclist->lock); //LOCK

        list_push_back(&wclist->lst, &newWC->elem);

        pthread_mutex_unlock(&wclist->lock);
        return newWC;
    }
}

void fprint_words(word_count_list_t *wclist, FILE *outfile) {
    struct list_elem *wc = list_begin(&wclist->lst);
    pthread_mutex_lock(&wclist->lock); //LOCK
    for (wc = list_begin(&wclist->lst); wc != list_end(&wclist->lst); wc = list_next(wc)) {
        word_count_t *element = list_entry(wc, word_count_t, elem);
        fprintf(outfile, "%8d\t%s\n", element->count, element->word);
    }
    pthread_mutex_unlock(&wclist->lock);
}

void wordcount_sort(word_count_list_t *wclist,
                    bool less(const word_count_t *, const word_count_t *)) {
    //list_sort(wclist, less, NULL);
    return;
}
