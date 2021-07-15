#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <string.h>
#include <limits.h>
#define _GNU_SOURCE
#include <pthread.h>
#define numPrioridades 8

char *nivelPrioridad[8] = {"emerg", "alert", "crit", "err", "warning", "notice", "info", "debug"};

typedef struct Prioridades{
    int nivel;
    char servicio[500];
} Prioridad;

char *concatenate(const char *s1, const char *s2, const char *s3, const char *s4, const char *s5) {
    size_t len1 = strlen(s1);
    size_t len2 = strlen(s2);
    size_t len3 = strlen(s3);
    size_t len4 = strlen(s4);
    size_t len5 = strlen(s5);
    char *res = malloc(len1 + len2 + len3 + len4 + len5 + 1);
    if (res) {
        memcpy(res, s1, len1);
        memcpy(res + len1, s2, len2);
        memcpy(res + len1 + len2 , s3, len3);
        memcpy(res + len1 + len2 + len3, s4, len4);
        memcpy(res + len1 + len2 + len3 + len4, s5, len5 + 1);
    }
    return res;
}

void* crearPrioridad(void* prioridadData) {
    Prioridad *data = (Prioridad *)prioridadData;
    char *s1 = "journalctl -u "; 
	char *s2 = data->servicio;
	char *s3 = " -o cat -p "; 
	char s4[20];
	sprintf(s4, "%d", data->nivel);
	char *s5 = "| wc -c"; 
	char *comando = concatenate(s1, s2, s3, s4, s5);
    FILE *p = popen(comando, "r");
    free(comando);
    free(prioridadData);
    return (void *)p;

}

FILE *crearServicio(const char *servicio) {
    
    pthread_t th[numPrioridades];
    int i;
    pthread_mutex_t lock;
    if (pthread_mutex_init(&lock, NULL) != 0)
    {
        perror("mutex init failed");
    }
    for (i = 0; i < numPrioridades; i++) {
         
        Prioridad *prioridadData = (Prioridad *)malloc(sizeof(Prioridad));
        if(prioridadData == NULL){
            perror("Error creando malloc de prioridad");
        }
        strcpy(prioridadData->servicio,servicio);
        prioridadData->nivel = i;
        if (pthread_create(&th[i], NULL, crearPrioridad, (void*) prioridadData) != 0) {
            perror("Error creando el hilo");
        }
    }
        FILE *fServicio;
        char *header= "--------------------\n";
        
        if ((fServicio = fopen("servicios.txt","a")) == NULL){
                perror("No se puso arbir el archivo de servicio");
        }
        fprintf(fServicio, "%s" ,header);
        fprintf(fServicio, "%s" ,servicio);
        fprintf(fServicio, "\n");
        fprintf(fServicio, "%s" ,header);

        for (i = 0; i < numPrioridades; i++) {
            FILE *fPrioridad;

            
            
            if (pthread_join(th[i], (void **) &fPrioridad) != 0) {
                perror("Error uniendo el hilo");
            }
            if( fPrioridad == NULL)
            {
                perror("No se pudo abrir el archivo");
            }
            
            int BUFF_SIZE = 1024;
            int size_line; 
            char line[BUFF_SIZE];
            
            char *resultado = (char*) malloc(BUFF_SIZE * sizeof(char));
            
            
            char *nomPrioridad = nivelPrioridad[i]; 
            char *espacios = " :";
            char *numMensajes;
            if(fgets(line, size_line = sizeof(line), fPrioridad) != NULL){
                numMensajes = line;
            }
            
            pthread_mutex_lock(&lock);
            strcat(resultado, nomPrioridad);
            strcat(resultado, espacios);
            strcat(resultado, numMensajes);
            pclose(fPrioridad);
            fprintf(fServicio,"%s",resultado);
            pthread_mutex_unlock(&lock);
            free(resultado);
        }
        pthread_mutex_destroy(&lock);
        return fServicio;
    
}


int main(int argc, char **argv) {

    FILE *fServicio;
    if ((fServicio = fopen("servicios.txt","w")) == NULL){
        perror("No se puso arbir el archivo de servicio");
    }
    fprintf(fServicio, "Log Program\n");
    fclose(fServicio);
    system("clear");
    char str1[10000];
    FILE *p;
    char ch;
    while (fgets(str1, sizeof str1, stdin) == str1 && strcmp(str1, "exit\n") != 0){
        str1[strcspn(str1, "\n")] = 0;
        int wstatus;
        pid_t ch_pid = fork();

        
        if (ch_pid == -1) {
            perror("fork");
            exit(EXIT_FAILURE);
        }

        if (ch_pid == 0) {
            
            p = crearServicio(str1);
            if( p == NULL)
            {
                puts("No se pudo abrir el archivo");
                return(1);
            }
            while( (ch=fgetc(p)) != EOF)
                putchar(ch);
            pclose(p);
            exit(EXIT_SUCCESS);
        }

        
        if (waitpid(ch_pid, &wstatus, WUNTRACED | WCONTINUED) == -1) {
            perror("waitpid");
            exit(EXIT_FAILURE);
        }
        

        
    }
    exit(EXIT_SUCCESS);
    return 0;
}
