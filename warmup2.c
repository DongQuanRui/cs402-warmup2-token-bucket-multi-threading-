#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
//#include <time.h>
#include <sys/time.h>

#include "my402list.h"
#include "cs402.h"

typedef struct tagPacket{
    int index;
    int token_required;
}packetElem;

//initialize the global variables for this simulation
My402List Q1;
My402List Q2;
int token = 0;
int total_num_packet = 0;
pthread_mutex_t IOlock;

//initialize default parameters
int P = 0, num = 0, B = 0;
double lambda = 0.00, mu = 0.00, r = 0.00;

//time processing
double start_time = 0;
double proc_time = 0;
int digit = 0;

double in_Q1_time_before = 0;
double in_Q1_time_after = 0;
double in_Q1_time = 0;
double in_Q2_time_before = 0;
double in_Q2_time_after = 0;
double in_Q2_time = 0;


double get_wall_time()
{
    struct timeval time ;
    if (gettimeofday(&time,NULL)){
        return 0;
    }
    //    cout << (double)time.tv_sec << " "  << (double)time.tv_usec * .000001 << endl;
    return (double)time.tv_sec + (double)time.tv_usec * .000001;
}

void *packet_proc(void *id)
{
    double inter_arrival_time_before = start_time;
    double inter_arrival_time_after = 0;
    double inter_arrival_time = 0;
//    double in_Q1_time_before = 0;
//    double in_Q1_time_after = 0;
//    double in_Q1_time = 0;
//    double in_Q2_time_before = 0;
//    double in_Q2_time_after = 0;
//    double in_Q2_time = 0;
    
//    while (1) {
    while (total_num_packet < num) {
        //sleep for an interval rate = lambda
        usleep(lambda * 1000000);
        
        //wake up, create a packet object
        total_num_packet++;
        
        pthread_mutex_lock(&IOlock);
        
        //check if packet needs to be dropped
        if (P <= B) {//if the packet needs to be enqueued
            //create a packet
            packetElem *packet = (packetElem *)malloc(sizeof(packetElem));
            packet->index = total_num_packet;
            packet->token_required = P;
            
            //print time (????????:???ms: p? arrives, needs ? tokends, inter-arrival time = ?ms)
            inter_arrival_time_after = get_wall_time();
            proc_time = inter_arrival_time_after - start_time;
            digit = (int)proc_time;
            
            char str_digit[8];
            sprintf(str_digit, "%d", digit);
            for (int i=0; i<8-strlen(str_digit); i++) {
                printf("0");
            }
            printf("%.3fms: ", proc_time);
            
            
            inter_arrival_time = inter_arrival_time_after - inter_arrival_time_before;
            printf("p%d arrives, need %d tokens, inter-arrival time = ", total_num_packet, P);
            for (int i=0; i<8-strlen(str_digit); i++) {
                printf("0");
            }
            printf("%.3fms\n", inter_arrival_time);
            inter_arrival_time_before = inter_arrival_time_after;
        
            //enqueue this packet to Q1
            My402ListPrepend(&Q1, packet);
            in_Q1_time_before = get_wall_time();
            
            //print time (????????:???ms: p? enters Q1)
            proc_time = in_Q1_time_before - start_time;
            digit = (int)proc_time;
            sprintf(str_digit, "%d", digit);
            for (int i=0; i<8-strlen(str_digit); i++) {
                printf("0");
            }
            printf("%.3fms: ", proc_time);
            printf("p%d enters Q1\n", total_num_packet);
            
        }else{//if packet is dropped
            //print time (????????:???ms: p? arrives, needs ? tokends, inter-arrival time = ?ms)
            inter_arrival_time_after = get_wall_time();
            proc_time = inter_arrival_time_after - start_time;
            digit = (int)proc_time;
            
            char str_digit[8];
            sprintf(str_digit, "%d", digit);
            for (int i=0; i<8-strlen(str_digit); i++) {
                printf("0");
            }
            printf("%.3fms: ", proc_time);
            
            
            inter_arrival_time = inter_arrival_time_after - inter_arrival_time_before;
            printf("p%d arrives, need %d tokens, inter-arrival time = ", total_num_packet, P);
            for (int i=0; i<8-strlen(str_digit); i++) {
                printf("0");
            }
            printf("%.3fms, dropped\n", inter_arrival_time);
            inter_arrival_time_before = inter_arrival_time_after;
            
        }
        
        //check if the packet at the head of Q1 can be moved into Q2
        My402ListElem *temp;
        if (!My402ListEmpty(&Q1)) {
            temp = My402ListLast(&Q1);
        }else{
            pthread_mutex_unlock(&IOlock);
            continue;
        }
        
        packetElem *packet = (packetElem *)(temp->obj);
        if (packet->token_required <= token) {
            My402ListUnlink(&Q1, temp);
            token = token - packet->token_required;
            
            //print time(????????:???ms: p? leaves Q1, time in Q1 = ?ms, token bucket now has ? token)
            in_Q1_time_after = get_wall_time();
            in_Q1_time = in_Q1_time_after - in_Q1_time_before;
            
            proc_time = get_wall_time() - start_time;
            digit = (int)proc_time;
            
            char str_digit[8];
            sprintf(str_digit, "%d", digit);
            for (int i=0; i<8-strlen(str_digit); i++) {
                printf("0");
            }
            printf("%.3fms: ", proc_time);
            
            if (token <= 1) {
                printf("P%d leaves Q1, time in Q1 = %.3fms, token bucket now has %d token\n", total_num_packet, in_Q1_time, token);
            }else{
                printf("P%d leaves Q1, time in Q1 = %.3fms, token bucket now has %d tokens\n", total_num_packet, in_Q1_time, token);
            }
            
            //packet enters Q2
            My402ListPrepend(&Q2, temp);
            in_Q2_time_before = get_wall_time();
            
            //print time(????????:???ms: p? enters Q2)
            proc_time = in_Q2_time_before - start_time;
            digit = (int)proc_time;
            sprintf(str_digit, "%d", digit);
            for (int i=0; i<8-strlen(str_digit); i++) {
                printf("0");
            }
            printf("%.3fms: ", proc_time);
            printf("p%d enters Q2\n", total_num_packet);
            
            
        
        }
        
        pthread_mutex_unlock(&IOlock);
    }
    pthread_exit(NULL);
}


void *token_proc(void * end_time)
{
    int total_num_token = 0;
//    double in_Q1_time_before = 0;
//    double in_Q1_time_after = 0;
//    double in_Q1_time = 0;
//    double in_Q2_time_before = 0;
//    double in_Q2_time_after = 0;
//    double in_Q2_time = 0;

//    while (1) {
    while (total_num_packet < num) {
        //sleep for an interval (rate r)
        usleep(r * 1000000);
        
        //wake up
        pthread_mutex_lock(&IOlock);
        total_num_token++;
        
        //
        if (token < B) {//if token bucket is not full, increment token count
            token++;
//            total_num_token++;
            
            //print time (????????:???ms: token t? arrives, tokens bucket now has ? token)
            proc_time = get_wall_time() - start_time;
            digit = (int)proc_time;
            char str_digit[8];
            sprintf(str_digit, "%d", digit);
            for (int i=0; i<8-strlen(str_digit); i++) {
                printf("0");
            }
            printf("%.3fms: ", proc_time);
            
            //plural form or not
            if (token == 1) {
                printf("token t%d arrives, token bucket now has 1 token\n", total_num_token);
            }else{
                printf("token t%d arrives, token bucket now has %d tokens\n", total_num_token, token);
            }
            
            //check if token thread can move one packet from Q1 to Q2
            My402ListElem *temp;
            if (!My402ListEmpty(&Q1)) {
                temp = My402ListLast(&Q1);
            }else{
                pthread_mutex_unlock(&IOlock);
                continue;
            }
            
            packetElem *packet = (packetElem *)(temp->obj);
            if (packet->token_required <= token) {//if packet can be moved from Q1 to Q2
                My402ListUnlink(&Q1, temp);
                token = token - packet->token_required;
                
                //print time(????????:???ms: p? leaves Q1, time in Q1 = ?ms, token bucket now has ? token)
                in_Q1_time_after = get_wall_time();
                in_Q1_time = in_Q1_time_after - in_Q1_time_before;
                
                proc_time = get_wall_time() - start_time;
                digit = (int)proc_time;
                
                char str_digit[8];
                sprintf(str_digit, "%d", digit);
                for (int i=0; i<8-strlen(str_digit); i++) {
                    printf("0");
                }
                printf("%.3fms: ", proc_time);
                
                if (token <= 1) {
                    printf("P%d leaves Q1, time in Q1 = %.3fms, token bucket now has %d token\n", total_num_packet, in_Q1_time, token);
                }else{
                    printf("P%d leaves Q1, time in Q1 = %.3fms, token bucket now has %d tokens\n", total_num_packet, in_Q1_time, token);
                }
                
                //packet enters Q2
                My402ListPrepend(&Q2, temp);
                in_Q2_time_before = get_wall_time();
                
                //print time(????????:???ms: p? enters Q2)
                proc_time = in_Q2_time_before - start_time;
                digit = (int)proc_time;
                sprintf(str_digit, "%d", digit);
                for (int i=0; i<8-strlen(str_digit); i++) {
                    printf("0");
                }
                printf("%.3fms: ", proc_time);
                printf("p%d enters Q2\n", total_num_packet);
                
            }
        
            
        }else{//if token bucket is not full, increment token count
            
            //print time (????????:???ms: token t? arrives, dropped)
            proc_time = get_wall_time() - start_time;
            digit = (int)proc_time;
            char str_digit[8];
            sprintf(str_digit, "%d", digit);
            for (int i=0; i<8-strlen(str_digit); i++) {
                printf("0");
            }
            printf("%.3fms: token t%d arrives, dropped\n", proc_time, total_num_token);
        }
        
        /////////
        
        /////////
        
        
        
        pthread_mutex_unlock(&IOlock);
    }
    pthread_exit(NULL);
}

static
void Usage()
{
    fprintf(stderr,
            "Usage: warmup2 [-lambda lambda] [-mu mu] [-r r] [-B B] [-P P] [-n n] [-t tsfile]\n");
    exit(-1);
}

int main (int argc, char *argv[])
{
    FILE *fp = NULL;
    //number of parameters
    int num_lambda = 0, num_P = 0, num_num = 0, num_B = 0, num_mu = 0, num_r = 0;
    char filename[20];
    
    //recognize parameters
    for (argc--, argv++; argc > 0; argc--, argc--, argv++) {
        //check if file name specified
        if (strcmp(*argv, "-t") == 0) {
            argv++;
            fp = fopen(*argv, "r");
            strncpy(filename, *argv, strlen(*argv));
        }
        //check if lambda is input
        else if (strcmp(*argv, "-lambda") == 0) {
            num_lambda = 1;
            argv++;
            if (atof(*argv) > 10) {
                lambda = 10;
            }else if(atof(*argv) < 0){
                Usage();
            }else{
                lambda = atof(*argv);
            }
        }
        //chech if mu is input
        else if(strcmp(*argv, "-mu") == 0){
            num_mu = 1;
            argv++;
            if (atof(*argv) > 10) {
                mu = 10;
            }else if(atof(*argv) < 0){
                Usage();
            }else{
                mu = atof(*argv);
            }
        }
        //check if r is input
        else if(strcmp(*argv, "-r") == 0){
            num_r = 1;
            argv++;
            if (atof(*argv) > 10) {
                r = 10;
            }else if(atof(*argv) < 0){
                Usage();
            }else{
                r = atof(*argv);
            }
        }
        //check if P is input
        else if(strcmp(*argv, "-P") == 0){
            num_P = 1;
            argv++;
            if (atoi(*argv) > 10) {
                P = 10;
            }else if(atoi(*argv) < 0){
                Usage();
            }else{
                P = atoi(*argv);
            }
        }
        //check if num is input
        else if(strcmp(*argv, "-n") == 0){
            num_num = 1;
            argv++;
            if (atoi(*argv) > 10) {
                num = 10;
            }else if(atoi(*argv) < 0){
                Usage();
            }else{
                num = atoi(*argv);
            }
        }
        //check if B is input
        else if(strcmp(*argv, "-B") == 0){
            num_B = 1;
            argv++;
            if (atoi(*argv) > 10) {
                B = 10;
            }else if(atoi(*argv) < 0){
                Usage();
            }else{
                B = atoi(*argv);
            }
        }
    }
    
    //default value of parameters
    if (fp == NULL) {
        if (num_lambda == 0)
            lambda = 1.00;
        if (num_mu == 0)
            mu = 0.35;
        if (num_r == 0)
            r = 1.5;
        if (num_P == 0)
            P = 3;
        if (num_B == 0)
            B = 10;
        if (num_num == 0)
            num = 20;
    }else{
        //read file
        
        //
        fclose(fp);
    }
    
    printf("Emulation Parameters:\n");
    printf("    number to arrive = %d\n", num);
    printf("    lambda = %.2f\n", lambda);
    printf("    mu = %.2f\n", mu);
    printf("    r = %.2f\n", r);
    printf("    P = %d\n", P);
    printf("    B = %d\n", B);
    if (fp != NULL) {
        printf("    tsfile = %s", filename);
    }
    printf("\n");
    
    
    //intialize the simulation(global varialbes, etc)
    memset(&Q1, 0, sizeof(My402List));
    memset(&Q2, 0, sizeof(My402List));
    (void)My402ListInit(&Q1);
    My402ListInit(&Q2);
    
    //Emulation begin
    start_time = get_wall_time();
    proc_time = get_wall_time() - start_time;
    digit = (int)proc_time;
    char str_digit[8];
    sprintf(str_digit, "%d", digit);
    for (int i=0; i<8-strlen(str_digit); i++) {
        printf("0");
    }
    printf("%.3fms: emulation begins\n", proc_time);
    
    //create 4 threads
    pthread_t packet_thread, token_thread;// server1_thread, server2_thread;

    //packet arrival thread
    pthread_create(&packet_thread, NULL, packet_proc, (void *) (int) num);
    //token deposting thread
    pthread_create(&token_thread, NULL, token_proc, (void *) (int) num);
    //server thread
//    pthread_create(&server1_thread, NULL, producer, (void *) (int) i);
//    pthread_create(&server2_thread, NULL, producer, (void *) (int) i);
    
    ////////////////////////////////////////////////////////////////////////////////////
    
    
    ////////////////////////////////////////////////////////////////////////////////////
    
    //wait for thread to end
    pthread_join(packet_thread, NULL);
    pthread_join(token_thread, NULL);
//    pthread_join(server1_thread, NULL);
//    pthread_join(server2_thread, NULL);
    
    
    
    //print time(????????.???ms: emulation ends)
    proc_time = get_wall_time() - start_time;
    digit = (int)proc_time;
    
    str_digit[8];
    sprintf(str_digit, "%d", digit);
    for (int i=0; i<8-strlen(str_digit); i++) {
        printf("0");
    }
    printf("%.3fms: emulation ends\n", proc_time);
    
    return 0;
}






