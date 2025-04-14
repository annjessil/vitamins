/*
 * Implementation of the word_count interface using Pintos lists.
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
//#error "PINTOS_LIST must be #define'd when compiling word_count_l.c"
#endif

#include "word_count.h"

void init_words(word_count_list_t *wclist) {
    list_init(wclist); 
}

size_t len_words(word_count_list_t *wclist) {
    return list_size(&(*wclist));
}

word_count_t *find_word(word_count_list_t *wclist, char *word) {
    struct list_elem *wc = list_begin(wclist);
    for (wc = list_begin(wclist); wc != list_end(wclist); wc = list_next(wc)){
        word_count_t *element = list_entry(wc, word_count_t, elem);
        if (strcmp(word, element->word) == 0){
            return element;
        }
    }

    return NULL;
}

word_count_t *add_word_with_count(word_count_list_t *wclist, char *word,
                                  int count) {
    word_count_t *wcExisting = find_word(wclist, word);
    if (wcExisting != NULL){
        wcExisting->count = wcExisting->count + count;
        return wcExisting;
       
    } else {
        word_count_t *newWC = malloc(sizeof(word_count_t));
        if (newWC == NULL){
            return NULL; //in case malloc doesnt work
        }

        newWC->count = count;
        newWC->word = word; //strdup(word), if having issues
        list_push_back(wclist, &newWC->elem);
        return newWC;
    } 

    
}

word_count_t *add_word(word_count_list_t *wclist, char *word) {
    /*
 * Insert word with count=1, if not already present; increment count if
 * present. Takes ownership of word.
 */
    word_count_t *wc = find_word(wclist, word);
    if (wc != NULL){
        wc->count = wc->count + 1;
        return wc;
    } else {
        return add_word_with_count(wclist, word, 1);
    }
}

void fprint_words(word_count_list_t *wclist, FILE *outfile) {
    struct list_elem *wc = list_begin(wclist);
    for (wc = list_begin(wclist); wc != list_end(wclist); wc = list_next(wc)) {
        word_count_t *element = list_entry(wc, word_count_t, elem);
        fprintf(outfile, "%8d\t%s\n", element->count, element->word);
    }
}

static bool less_list(const struct list_elem *ewc1,
                      const struct list_elem *ewc2, void *aux) {
    //if first entry is less than second returns true               
    word_count_t *elem1 = list_entry(ewc1, word_count_t, elem);
    word_count_t *elem2 = list_entry(ewc2, word_count_t, elem);

    if (elem1->count < elem2->count){
        return true;
    } else if (elem1->count == elem2->count){
        if(strcmp(elem1->word, elem2->word) < 0){
            return true;
        } 
    } 

    return false;

}


void wordcount_sort(word_count_list_t *wclist, bool less(const word_count_t *, const word_count_t *)) {
    //sorting by word or count??
    //placeholder need to figure out how to actually do this lol
    list_sort(wclist, less_list, NULL);
}

