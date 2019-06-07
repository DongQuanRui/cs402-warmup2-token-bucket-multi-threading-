#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <math.h>
#include <pthread.h>
#include <sys/time.h>

#include "my402list.h"
#include "cs402.h"

typedef struct tagPacket{
    int index;
    int token_required;
    int S_time_read;
    
    double Q1_time;
    double Q2_time;
    double S_time;
    double time_in_system;
}packetElem;

//initialize the global variables for this simulation
My402List Q1;
My402List Q2;
int num_file = 0;
int token = 0;
int total_num_token = 0;
int total_num_packets_arrived = 0;
int total_num_packets_served = 0;
int total_num_packets_to_serve = 0;
int not_stop = 1;
FILE *fp = NULL;

//create 4 threads
pthread_t packet_thread, token_thread, server1_thread, server2_thread;

pthread_mutex_t IOlock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t Q_not_empty = PTHREAD_COND_INITIALIZER;

//statstics
double sum_inter_arrival_time = 0;
double sum_service_time = 0;
double sum_time_in_system = 0;
double avg_num_in_Q1 = 0;
double avg_num_in_Q2 = 0;
double avg_num_at_S1 = 0;
double avg_num_at_S2 = 0;
int num_packets_dropped = 0;
int num_token_dropped = 0;

//initialize default parameters
int P, num, B;
double lambda, mu, r;

//time processing
double start_time = 0;
double end_time = 0;
double proc_time = 0;
int digit = 0;

double in_Q1_time_before = 0;
double in_Q1_time_after = 0;
double in_Q1_time = 0;
double in_Q2_time_before = 0;
double in_Q2_time_after = 0;
double in_Q2_time = 0;
double in_S_time_before = 0;
double in_S_time_after = 0;
double in_S_time = 0;

double *storage;
double *backup_storage;
int size = 20;

void create_storage(int size){
    backup_storage = (double *)malloc((size-20) * sizeof(double));
    for (int i=0; i<(size-20); i++) {
        backup_storage[i] = storage[i];
    }
    free(storage);
    storage = (double *)malloc(size * sizeof(double));
    for (int i=0; i<(size-20); i++) {
        storage[i] = backup_storage[i];
    }
    free(backup_storage);
}

double get_wall_time()
{
    struct timeval time ;
    if (gettimeofday(&time,NULL)){
        return 0;
    }
    return (double)time.tv_sec * 1000 + (double)time.tv_usec * .001;
}

void statstics(){
    printf("\nStatistics:\n\n");
    printf("    average packet inter-arrival time = %.6f\n", (sum_inter_arrival_time / num));
    printf("    average packet service time = %.6f\n\n", (sum_service_time / total_num_packets_to_serve));
    
    printf("    average number of packets in Q1 = %.6f\n", (avg_num_in_Q1 / end_time));
    printf("    average number of packets in Q2 = %.6f\n", (avg_num_in_Q2 / end_time));
    printf("    average number of packets at S1 = %.6f\n", (avg_num_at_S1 / end_time));
    printf("    average number of packets at S2 = %.6f\n\n", (avg_num_at_S2 / end_time));
    
    double average_time_in_system = sum_time_in_system / total_num_packets_to_serve;
    printf("    average time a packet spent in system = %.6f\n", average_time_in_system);
    
    unsigned int diff_sqrt_sum = 0;
    for (int i=0; i<total_num_packets_served; i++) {
        //        printf("%.2f\n", storage[i]);
        diff_sqrt_sum += abs((storage[i] - average_time_in_system) * (storage[i] - average_time_in_system));
    }
    //    printf("    diff_sqrt_sum = %d\n\n", diff_sqrt_sum);
    printf("    standard deviation for time spent in system = %.6f\n\n", sqrt(diff_sqrt_sum / total_num_packets_served));
    
    printf("    token drop probability = %.2f\n", ((double)num_token_dropped / (double)total_num_token));
    printf("    packet drop probability = %.2f\n", ((double)num_packets_dropped / (double)total_num_packets_arrived));
}

void print_time(double proc_time){
    digit = (int)proc_time;
    char str_digit[8];
    sprintf(str_digit, "%d", digit);
    for (int i=0; i<8-strlen(str_digit); i++) {
        printf("0");
    }
    printf("%.3fms: ", proc_time);
}

void *packet_proc(void *fp)
{
    double inter_arrival_time_before = start_time;
    double inter_arrival_time_after = 0;
    double inter_arrival_time = 0;
    
    //loop
    for (; total_num_packets_arrived < num; ) {
        ///////////////////////////////    Q1    ///////////////////////////////
        //read info from file
        if (num_file == 1) {
            ////
            char buffer[1024];
            fgets(buffer, sizeof(buffer), fp);
//            printf("%s", buffer);
            char *start_ptr = buffer;
            char *seperate_ptr = strchr(start_ptr, ' ');
            if (seperate_ptr != NULL) {
                *seperate_ptr++ = '\0';
            }
            lambda = 1 / atof(start_ptr);
//            printf("lambda = %f     ", lambda);
            
            while (*seperate_ptr == ' ') {
                seperate_ptr++;
            }
            start_ptr = seperate_ptr;
            seperate_ptr = strchr(start_ptr, ' ');
            if (seperate_ptr != NULL) {
                *seperate_ptr++ = '\0';
            }
            P = atoi(start_ptr);
//            printf("P = %d     ", P);
            
            while (*seperate_ptr == ' ') {
                seperate_ptr++;
            }
            start_ptr = seperate_ptr;
            mu = 1 / atof(start_ptr);
//            printf("mu = %f\n", mu);
            /////////////////////
//            fscanf(fp, "%lf %d %lf", &lambda, &P, &mu);
            total_num_packets_arrived++;
        }else{
            total_num_packets_arrived++;
            if (total_num_packets_arrived == 1) {
                lambda = (1 / lambda) * 1000;
                mu = (1 / mu) * 1000;
            }
        }
        
        if (total_num_packets_arrived <= num) {
            //            printf("total num packets arrived: %d   num: %d\n", total_num_packets_arrived , num);
            //sleep for an interval rate = lambda
            usleep(lambda * 1000);
            
            //wake up, create a packet object
            packetElem *packet = (packetElem *)malloc(sizeof(packetElem));
            
            packet->index = total_num_packets_arrived;
            packet->token_required = P;
            packet->S_time_read = mu;
            
            pthread_mutex_lock(&IOlock);
            //////////////////////////    time    //////////////////////////
            //print time (????????:???ms: )
            inter_arrival_time_after = get_wall_time();
            proc_time = inter_arrival_time_after - start_time;
            
            print_time(proc_time);
            
            //print (p? arrives, needs ? tokends, inter-arrival time = ?ms)
            inter_arrival_time = inter_arrival_time_after - inter_arrival_time_before;
            sum_inter_arrival_time += inter_arrival_time;
            printf("p%d arrives, need %d tokens, inter-arrival time = %.3fms", total_num_packets_arrived, P, inter_arrival_time);
            inter_arrival_time_before = inter_arrival_time_after;
            
            //check if need to drop
            if (P <= B) {//if the packet needs to be enqueued
                printf("\n");
                
                //enqueue this packet to Q1
                in_Q1_time_before = get_wall_time();
                packet->Q1_time = in_Q1_time_before;
                My402ListPrepend(&Q1, packet);
                
                //////////////////////////    time    //////////////////////////
                //print time (????????:???ms: p? enters Q1)
                proc_time = in_Q1_time_before - start_time;
                print_time(proc_time);
                printf("p%d enters Q1\n", total_num_packets_arrived);
            }else{//if packet is dropped
                //print time (, dropped)
                printf(", dropped\n");
                total_num_packets_to_serve--;
                num_packets_dropped++;
                //////////////////////////    time    //////////////////////////
            }
            pthread_mutex_unlock(&IOlock);
            packet = NULL;
            free(packet);
            
            ///////////////////////////////    Q1    ///////////////////////////////
            
            ///////////////////////////////    Q1->Q2    ///////////////////////////////
            //managing Q2
            pthread_mutex_lock(&IOlock);
            
            //check if the packet at the head of Q1 can be moved into Q2
            My402ListElem *temp = NULL;
            if (!My402ListEmpty(&Q1)) {
                temp = My402ListLast(&Q1);
            }else{
                pthread_mutex_unlock(&IOlock);
                continue;
            }
            
            //packetElem *packet = (packetElem *)(temp->obj);
            packet = (packetElem *)(temp->obj);
            if (packet->token_required <= token) {
                My402ListUnlink(&Q1, temp);
                free(temp);
                token = token - packet->token_required;
                
                //////////////////////////    time    //////////////////////////
                //print time (????????:???ms: )
                in_Q1_time_after = get_wall_time();
                in_Q1_time = in_Q1_time_after - packet->Q1_time;
                
                proc_time = get_wall_time() - start_time;
                print_time(proc_time);
                
                //print time (p? leaves Q1, time in Q1 = ?ms, token bucket now has ? token)
                if (token <= 1) {
                    printf("P%d leaves Q1, time in Q1 = %.3fms, token bucket now has %d token\n", packet->index, in_Q1_time, token);
                }else{
                    printf("P%d leaves Q1, time in Q1 = %.3fms, token bucket now has %d tokens\n", packet->index, in_Q1_time, token);
                }
                packet->Q1_time = in_Q1_time;
                //////////////////////////    time    //////////////////////////
                
                //packet enters Q2
                in_Q2_time_before = get_wall_time();
                packet->Q2_time = in_Q2_time_before;
                My402ListPrepend(&Q2, packet);
                
                //////////////////////////    time    //////////////////////////
                //print time(????????:???ms: p? enters Q2)
                proc_time = in_Q2_time_before - start_time;
                print_time(proc_time);
                
                printf("p%d enters Q2\n", total_num_packets_arrived);
                //////////////////////////    time    //////////////////////////
                
                //broadcast the not empty signal to 2 servers
                if (!My402ListEmpty(&Q2)) {
                    pthread_cond_broadcast(&Q_not_empty);
                }
                ///////////////////////////////    Q1->Q2    ///////////////////////////////
            }
            packet = NULL;
            free(packet);
            
            pthread_mutex_unlock(&IOlock);
        }
    }
    pthread_exit(NULL);
}

void *token_proc()
{
    while (not_stop) {
        ///////////////////////////////    token    ///////////////////////////////
        //sleep for an interval (rate r)
        usleep((1 / r) * 1000000);
        
        //wake up and create a token
        pthread_mutex_lock(&IOlock);
        total_num_token++;
        
        //////////////////////////    time    //////////////////////////
        //print time (????????:???ms: )
        proc_time = get_wall_time() - start_time;
        print_time(proc_time);
        
        //check if token bucket full or not
        if(token < B) {//if token bucket is not full, increment token count
            token++;
            //print time (token t? arrives, tokens bucket now has ? token)
            if (token == 1) {
                printf("token t%d arrives, token bucket now has 1 token\n", total_num_token);
            }else{
                printf("token t%d arrives, token bucket now has %d tokens\n", total_num_token, token);
            }
        }else{//if token bucket is not full, increment token count
            //print time (token t? arrives, dropped)
            printf("token t%d arrives, dropped\n", total_num_token);
            num_token_dropped++;
        }
        //////////////////////////    time    //////////////////////////
        pthread_mutex_unlock(&IOlock);
        ///////////////////////////////    token    ///////////////////////////////
        
        ///////////////////////////////    Q1->Q2    ///////////////////////////////
        pthread_mutex_lock(&IOlock);
        
        //check if token thread can move one packet from Q1 to Q2
        My402ListElem *temp = NULL;
        if (!My402ListEmpty(&Q1)) {
            temp = My402ListLast(&Q1);
        }else{
            pthread_mutex_unlock(&IOlock);
            continue;
        }
        
        packetElem *packet = (packetElem *)(temp->obj);
        if (packet->token_required <= token) {//if packet can be moved from Q1 to Q2
            
            //pop this packet from Q1
            My402ListUnlink(&Q1, temp);
            token = token - packet->token_required;
            
            //////////////////////////    time    //////////////////////////
            //print time(????????:???ms: )
            in_Q1_time_after = get_wall_time();
            in_Q1_time = in_Q1_time_after - packet->Q1_time;//_before;
            
            proc_time = in_Q1_time_after - start_time;
            print_time(proc_time);
            
            //print time(p? leaves Q1, time in Q1 = ?ms, token bucket now has ? token)
            if (token <= 1) {
                printf("p%d leaves Q1, time in Q1 = %.3fms, token bucket now has %d token\n", packet->index, in_Q1_time, token);
            }else{
                printf("p%d leaves Q1, time in Q1 = %.3fms, token bucket now has %d tokens\n", packet->index, in_Q1_time, token);
            }
            packet->Q1_time = in_Q1_time;
            //////////////////////////    time    //////////////////////////
            
            //packet enters Q2
            in_Q2_time_before = get_wall_time();
            packet->Q2_time = in_Q2_time_before;
            My402ListPrepend(&Q2, packet);
            
            //////////////////////////    time    //////////////////////////
            //print time(????????:???ms:)
            proc_time = in_Q2_time_before - start_time;
            print_time(proc_time);
            
            //print time(p? enters Q2)
            printf("p%d enters Q2\n", packet->index);
            //////////////////////////    time    //////////////////////////
            
            pthread_cond_broadcast(&Q_not_empty);
        }
        ///////////////////////////////    Q1->Q2    ///////////////////////////////
        pthread_mutex_unlock(&IOlock);
    }
    pthread_exit(NULL);
}

void *server_proc(void *i){
    //loop: When not all the packets are processed
    while (not_stop) {
        ///////////////////////////////    before in S    ///////////////////////////////
        pthread_mutex_lock(&IOlock);
        while (My402ListEmpty(&Q2)) {
            if (!not_stop) {
                break;
            }
            pthread_cond_wait(&Q_not_empty, &IOlock);
        }
        
        //take the head packet in Q2 in either S1 or S2
        My402ListElem *temp = NULL;
        if (!My402ListEmpty(&Q2)) {
            temp = My402ListLast(&Q2);
        }else{
            pthread_mutex_unlock(&IOlock);
            continue;
        }
        
        packetElem *packet = (packetElem *)(temp->obj);
        My402ListUnlink(&Q2, temp);
        free(temp);
        
        //////////////////////////    time    //////////////////////////
        //print time(????????:???ms: )
        in_Q2_time_after = get_wall_time();
        in_Q2_time = in_Q2_time_after - in_Q2_time_before;
        
        proc_time = in_Q2_time_after - start_time;
        print_time(proc_time);
        
        //print time(p? leaves Q2, time in Q2 = ?ms)
        printf("p%d leaves Q2, time in Q2 = %.3fms\n", packet->index, in_Q2_time);
        packet->Q2_time = in_Q2_time;
        //////////////////////////    time    //////////////////////////
        
        ///////////////////////////////    Q2->S1    ///////////////////////////////
        //packet enters S1/S2
        in_S_time_before = get_wall_time();
        packet->S_time = in_S_time_before;
        
        //////////////////////////    time    //////////////////////////
        //print time(????????:???ms:)
        proc_time = in_S_time_before - start_time;
        print_time(proc_time);
        
        //print time(p? enters S1 or S2)
        printf("p%d begins service at S%d, requesting %dms of service\n", packet->index, (int)i, (int)(packet->S_time_read));
        //////////////////////////    time    //////////////////////////
        
        pthread_mutex_unlock(&IOlock);
        ///////////////////////////////    before in S    ///////////////////////////////
        
        //sleep (served in S)
        usleep(1000 * packet->S_time_read);
        
        ///////////////////////////////    Q2->S1    ///////////////////////////////
        pthread_mutex_lock(&IOlock);
        
        in_S_time_after = get_wall_time();
        packet->S_time = in_S_time_after - packet->S_time;
        packet->time_in_system = packet->Q1_time + packet->Q2_time + packet->S_time;
        avg_num_in_Q1 += packet->Q1_time;
        avg_num_in_Q2 += packet->Q1_time;
        if ((int)i == 1) {
            avg_num_at_S1 += packet->S_time;
        }else{
            avg_num_at_S2 += packet->S_time;
        }
        
        
        sum_service_time += packet->S_time;
        sum_time_in_system += packet->time_in_system;
        //////////////////////////    time    //////////////////////////
        //print time(????????:???ms:)
        proc_time = in_S_time_after - start_time;
        print_time(proc_time);
        
        printf("p%d departs from S%d, service time = %.3fms, time in system = %.3fms\n", packet->index, (int)i, packet->S_time, packet->time_in_system);
        total_num_packets_served++;
        //////////////////////////    time    //////////////////////////
        
        if (total_num_packets_served <= size) {
            storage[total_num_packets_served-1] = packet->time_in_system;
        }else{
            size = size + 20;
            create_storage(size);
            storage[total_num_packets_served-1] = packet->time_in_system;
        }
        
        //termination condition
        //        printf("In S1/S2, total num packets served is: %d, total num packet to serve is: %d\n", total_num_packets_served, total_num_packets_to_serve);
        if (total_num_packets_served == total_num_packets_to_serve) {
            //            printf("server1 prepare to terminate\n");
            not_stop = 0;
            pthread_cond_signal(&Q_not_empty);
            pthread_mutex_unlock(&IOlock);
            break;
        }
        pthread_mutex_unlock(&IOlock);
        ///////////////////////////////    Q2->S1    ///////////////////////////////
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
    //number of parameters
    int num_lambda = 0, num_P = 0, num_num = 0, num_B = 0, num_mu = 0, num_r = 0;
    char filename[20];
    
    //recognize parameters
    for (argc--, argv++; argc > 0; argc--, argc--, argv++) {
        //check if file name specified
        if (strcmp(*argv, "-t") == 0) {
            num_file++;
            argv++;
            fp = fopen(*argv, "r");
            strncpy(filename, *argv, strlen(*argv));
            filename[strlen(*argv)] = '\0';
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
            if (atoi(*argv) > 50) {
                B = 50;
            }else if(atoi(*argv) < 0){
                Usage();
            }else{
                B = atoi(*argv);
            }
        }
    }
    
    printf("Emulation Parameters:\n");
    //default value of parameters
    if (num_file == 0) {
        //        printf("default mode\n");
        if (num_num == 0)
            num = 20;
        printf("    number to arrive = %d\n", num);
        if (num_lambda == 0){
            lambda = 1;
            printf("    lambda = %.2f\n", lambda);
        }
        if (num_mu == 0){
            mu = 0.35;
            printf("    mu = %.2f\n", mu);
        }
        if (num_r == 0)
            r = 1.5;
        printf("    r = %.2f\n", r);
        if (num_B == 0)
            B = 10;
        printf("    B = %d\n", B);
        if (num_P == 0){
            P = 3;
            printf("    P = %d\n", P);
        }
        total_num_packets_to_serve = num;
    }else{
        //read file
        char buffer[1024];
        fgets(buffer, sizeof(buffer), fp);
        num = atoi(buffer);
        total_num_packets_to_serve = num;
        printf("    number to arrive = %d\n", num);
        if (num_r == 0)
            r = 1.5;
        printf("    r = %.2f\n", r);
        if (num_B == 0)
            B = 10;
        printf("    B = %d\n", B);
    }
    
    if (num_file == 1) {
        printf("    tsfile = %s\n", filename);
    }
    printf("\n");
    
    //intialize the simulation(global varialbes, etc)
    memset(&Q1, 0, sizeof(My402List));
    memset(&Q2, 0, sizeof(My402List));
    (void)My402ListInit(&Q1);
    (void)My402ListInit(&Q2);
    storage = (double *)malloc(size * sizeof(double));

    //Emulation begin
    start_time = get_wall_time();
    proc_time = start_time - start_time;
    digit = (int)proc_time;
    char str_digit[8];
    sprintf(str_digit, "%d", digit);
    for (int i=0; i<8-strlen(str_digit); i++) {
        printf("0");
    }
    printf("%.3fms: emulation begins\n", proc_time);
    
    //packet arrival thread
    pthread_create(&packet_thread, NULL, packet_proc, (void *) fp);
    //token deposting thread
    pthread_create(&token_thread, NULL, token_proc, NULL);
    //server thread
    pthread_create(&server1_thread, NULL, server_proc, (void *)1);
    pthread_create(&server2_thread, NULL, server_proc, (void *)2);
    
    ////////////////////////////////////////////////////////////////////////////////////
    //wait for thread to end
    //    if(pthread_join(packet_thread, NULL) == 0){
    //        printf("packet thread terminates\n");
    //    }
    //    if(pthread_join(token_thread, NULL) == 0){
    //        printf("token thread terminates\n");
    //    }
    //    if (pthread_join(server1_thread, NULL) == 0) {
    //        printf("server1 terminates\n");
    //    }
    //    if (pthread_join(server2_thread, NULL) == 0) {
    //        printf("server2 terminates\n");
    //    }
    pthread_join(packet_thread, NULL);
    pthread_join(token_thread, NULL);
    pthread_join(server1_thread, NULL);
    pthread_join(server2_thread, NULL);
    
    //print time(????????.???ms: emulation ends)
    end_time = get_wall_time() - start_time;
    print_time(end_time);
    printf("emulation ends\n");
    
    statstics();
    
    //clean up memory
    if (My402ListEmpty(&Q1)) {
        My402ListUnlinkAll(&Q1);
    }
    if (My402ListEmpty(&Q2)) {
        My402ListUnlinkAll(&Q2);
    }
    
    free(storage);
    return 0;
}



