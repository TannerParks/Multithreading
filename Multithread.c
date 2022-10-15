#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

//gcc --std=gnu99 -o multithread Multithread.c -lpthread

// I used the Condition Variables exploration for the global variables and the following replit links provided to us for the outline
// https://replit.com/@cs344/64prodconsunboundsolc#main.c
// https://replit.com/@cs344/65prodconspipelinec#main.c


// Buffer, shared resource (50 lines of 1000 characters each)
char buffer1[50][1000];
char buffer2[50][1000];
char buffer3[50][1000];

// Number of items in buffer, shared resource
int count1 = 0;
int count2 = 0;
int count3 = 0;

// Index where producer will pick up next item
int prod_idx1 = 0;
int prod_idx2 = 0;
int prod_idx3 = 0;

// Index where consumer will pick up next item
int con_idx1 = 0;
int con_idx2 = 0;
int con_idx3 = 0;

// Mutexes
pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex2 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex3 = PTHREAD_MUTEX_INITIALIZER;

// Condition variables (unbounded so we  only need full condition)
pthread_cond_t full1 = PTHREAD_COND_INITIALIZER;
pthread_cond_t full2 = PTHREAD_COND_INITIALIZER;
pthread_cond_t full3 = PTHREAD_COND_INITIALIZER;


// Put line in buffer 1
void put_buff_1(char line[]) {
    pthread_mutex_lock(&mutex1);

    strcpy(buffer1[prod_idx1], line);   // Copy to buffer1

    prod_idx1 = (prod_idx1 + 1) % 1000;
    count1++;
    
    pthread_cond_signal(&full1);    // Signal buffer isn't empty
    pthread_mutex_unlock(&mutex1);  // Unlock the mutex
}


void *get_input(void *args) {
    char input[1000];
    int stop_flag = 0;

    while (stop_flag != 1) {
        fgets(input, 1000, stdin);

        put_buff_1(input);  // Put input in buffer1

        if (strcmp(input, "STOP\n") == 0) { // We want stop in the buffer so break after
            stop_flag = 1;
        }
    }

    return NULL;
}


// Put line in buffer 2
void put_buff_2(char line[]) {
    pthread_mutex_lock(&mutex2);

    strcpy(buffer2[prod_idx2], line);   // Copy to buffer2

    prod_idx2 = (prod_idx2 + 1) % 1000;
    count2++;
    
    pthread_cond_signal(&full2);    // Signal buffer isn't empty
    pthread_mutex_unlock(&mutex2);  // Unlock the mutex
}


// Replace newline with space
void *sep_to_space(void *args) {
    char line[1000];

    for (int i=0; i < 50; i++) {
        
        // Consumer
        pthread_mutex_lock(&mutex1);

        while (count1 == 0) {
            pthread_cond_wait(&full1, &mutex1); // wait until buffer is full
        }

        strcpy(line, buffer1[con_idx1]);
        con_idx1 = (con_idx1 + 1) % 1000;
        count1--;

        pthread_mutex_unlock(&mutex1);

        if (strcmp(line, "STOP\n") == 0) { // break after stop
            put_buff_2(line);   // put in buffer2
            break;
        }
        else {
            line[strcspn(line, "\n")] = ' ';    // Change newline character to a space
            put_buff_2(line);  // put in buffer2
        }
    }

    return NULL;
}


// Put line in buffer 3
void put_buff_3(char line[]) {
    pthread_mutex_lock(&mutex3);

    strcpy(buffer3[prod_idx3], line);   // Copy to buffer3

    prod_idx3 = (prod_idx3 + 1) % 1000;
    count3++;
    
    pthread_cond_signal(&full3);    // Signal buffer isn't empty
    pthread_mutex_unlock(&mutex3);  // Unlock the mutex
}


// Replace ++ with ^
void *plus_to_caret(void *args) {
    char line[1000];

    for (int i=0; i < 50; i++) {

        // Consumer
        pthread_mutex_lock(&mutex2);

        while (count2 == 0) {
            pthread_cond_wait(&full2, &mutex2); // wait until buffer is full
        }

        strcpy(line, buffer2[con_idx2]);
        con_idx2 = (con_idx2 + 1) % 1000;
        count2--;

        pthread_mutex_unlock(&mutex2);

        // replace ++ with ^
        if (strcmp(line, "STOP\n") == 0) { // break after stop
            put_buff_3(line);
            break;
        }
        else {
            char *temp;

            for (int i = 0; i < strlen(line); i++) {   // Iterate through string
            
                if (line[i] == '+' && line[i+1] == '+') { // Check for back to back +
                    temp = strdup(line);
                    temp[i] = '%';  // Turns the ++ to %c
                    temp[i+1] = 'c';
                    sprintf(line, temp, '^');  // Saves temp to line variable
                }
            }

            put_buff_3(line);  // put in buffer3
        }

    }
    return NULL; 
}


// Output when 80 characters have been entered
void *output(void *args) {
    char line[1000];
    int counter = 0;
    char out_line[81];

    for (int i=0; i < 50; i++) {

        // Consumer
        pthread_mutex_lock(&mutex3);

        while (count3 == 0) {
            pthread_cond_wait(&full3, &mutex3); // wait until buffer is full
        }

        strcpy(line, buffer3[con_idx3]);
        con_idx3 = (con_idx3 + 1) % 1000;
        count3--;

        pthread_mutex_unlock(&mutex3);

        // ouput exactly 80 characters
        if (strcmp(line, "STOP\n") == 0) { // break after stop
            break;
        }
        else {
            for (int i=0; i < strlen(line); i++) {
                if (counter == 80) {
                    counter = 0;
                    printf("%s\n", out_line);
                    strcpy(out_line, "");
                }
                counter++;
                strncat(out_line, &line[i], 1);
            }
        }

    }
    return NULL; 
}


int main() {
    pthread_t in_thread;
    pthread_t line_sep;
    pthread_t plus_sign;
    pthread_t out_thread;

    pthread_create(&in_thread, NULL, get_input, NULL);
    pthread_create(&line_sep, NULL, sep_to_space, NULL);
    pthread_create(&plus_sign, NULL, plus_to_caret, NULL);
    pthread_create(&out_thread, NULL, output, NULL);

    pthread_join(in_thread, NULL);
    pthread_join(line_sep, NULL);
    pthread_join(plus_sign, NULL);
    pthread_join(out_thread, NULL);

    return 0;
}