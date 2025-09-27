/*Autor: Matias Muñoz Catalán. 
  Profesora: Victoria Flores.
  Curso: Sistenas Operativos.
  Descripción: Tarea busca implementar un programa en C que procesa un libro en formato de texto plano y construye un indice invertido utilizando procesamiento paralelo con la funcón fork() y comunicación entre procesos mediante pipe() */

// Se utilizan las siguientes librerias 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <ctype.h>

//Se definen constantes 

#define MAX_PALABRAS 1000       
#define MAX_LARGO_PALABRAS 100  
#define MAX_LARGO_PAGINA 100000 
#define MAX_PAGINAS 1000        
#define MAX_LARGO_TEXTO 5000000   

// Estructura para almacenar información de palabras
typedef struct {
    char palabra[MAX_LARGO_PALABRAS];
    int pagina;
    int contador;    
} infoPalabra;

// Estructura para almacenar palabras
typedef struct{
    char palabra[MAX_LARGO_PALABRAS];
    int paginas[MAX_PAGINAS];
    int contador[MAX_PAGINAS];
    int num_paginas;
} indice;

//Variables globales para las palabras de búsqueda 
char busca_palabras[MAX_PALABRAS][MAX_LARGO_PALABRAS];
int num_palabras = 0;

// Función para leer las palabras desde archivo
int abrir_archivo(const char* nomarchivo){
    FILE* archivo = fopen(nomarchivo, "r");
    if (!archivo){
        printf("ERROR al abrir archivo de palabras '%s'\n", nomarchivo); 
        return 0; 
    }
    char linea[MAX_LARGO_PALABRAS];
    num_palabras= 0;
   
    while (fgets(linea, sizeof(linea), archivo) && num_palabras < MAX_PALABRAS){
        //Eliminar salto de línea
        linea[strcspn(linea, "\n\r")]= '\0';
        //Convertir a minúsculas 
        for(int i=0; linea[i]; i++){
            linea[i]= tolower(linea[i]); 
        }
        //agregar si la palabra no está vacía
        if (strlen(linea)> 0){
            strcpy(busca_palabras[num_palabras], linea);
            num_palabras++;       
        }
    }
    fclose(archivo);
    printf(" %d palabras desde '%s'\n", num_palabras, nomarchivo);
    return 1;
}


// Funcion que va a leer el libro desde el archivo
char* leer_libro(const char* nomarchivo){
    FILE* archivo = fopen(nomarchivo, "r");
    if (!archivo){
        printf("Error al abrir archivo libro '%s'\n", nomarchivo);
        return NULL;    
    }
    //obtener tamaño del archivo
    fseek(archivo, 0, SEEK_END);
    long largo_archivo = ftell(archivo);
    fseek(archivo, 0, SEEK_SET);

    if(largo_archivo > MAX_LARGO_TEXTO){
       printf("Error el archivo es demasiado grande (%ld bytes). Máximo: %d bytes\n", largo_archivo, MAX_LARGO_TEXTO);
       fclose(archivo);
       return NULL;     
    }
    // Asignar memoria y leer el archivo
    char* contenido = malloc(largo_archivo + 1);
    if(!contenido){
        printf("Error no se puede asignar memoria para el libro\n");
        fclose(archivo);
        return NULL;
    }
    size_t largo_leido = fread(contenido, 1, largo_archivo, archivo);
    contenido[largo_leido] ='\0';
    
    fclose(archivo);
    printf("SE PUDO LEER EL LIRBO\n");
    return contenido;
}   

//Funcion para convertir texto a minusculas y limpiar caracteres especiales
void limpiar_texto(char* palabra){
    int largo = strlen(palabra);
    int j = 0;

    for(int i=0 ; i < largo; i++){
        if(isalnum(palabra[i])) {
            palabra[j++]= tolower(palabra[i]);        
        }    
    }
    palabra[j]= '\0';

}

//Funcion para verificar si una palabra esta en la lista de busqueda

int existe_palabra(const char* palabra){
    for (int i= 0; i < num_palabras; i++){
        if(strcmp(palabra, busca_palabras[i])== 0){
            return 1;        
        }    
    }
    return 0;
}

//Funcion para procesar una página y contar palabras

void procesar_pagina(int num_pagina, const char* texto_pagina, int escritura){
    infoPalabra contador_palabras[MAX_PALABRAS];

    //inicializar contadores para todas las palabras de busqueda
    for (int i= 0; i < num_palabras; i++){
        strcpy(contador_palabras[i].palabra, busca_palabras[i]);
        contador_palabras[i].pagina = num_pagina;
        contador_palabras[i].contador = 0;        
    }   
    
    //Crear una copia del texto para tokenizar.
    char* copia_texto = strdup(texto_pagina);
    char* token =strtok(copia_texto," \t\n\r.,;:!?()[]{}\"'-_=+*&%$#@<>/\\|`~" );
    
    while(token != NULL){
        limpiar_texto(token);
        if(strlen(token)> 0 && existe_palabra(token)){
            //Buscar la palabra en nuestro array
            for(int i=0; i< num_palabras; i++ ){
                if(strcmp(token,busca_palabras[i])==0){
                    contador_palabras[i].contador++;
                    break;  
                }
            }
        }
        token = strtok(NULL, " \t\n\r.,;:!?()[]{}\"'-_=+*&%$#@<>/\\|`~");
    }
    //enviar resultados al proceso padre (solo palabras con contador >0)
    for(int i=0; i < num_palabras; i++){
        if(contador_palabras[i].contador > 0){
            write(escritura, &contador_palabras[i], sizeof(infoPalabra));       
        }
    } 
    free(copia_texto);
    close(escritura);  
}

//funcion para comparar entradas del indice (para ordenamiento alfabetico).

int comparar_indice(const void* a, const void* b){
    indice* entrada_a =(indice*)a;
    indice* entrada_b =(indice*)b;
    return strcmp(entrada_a ->palabra, entrada_b ->palabra);   
}

//Funcion para imprimir el indice invertido
void print_indice_invertido(indice* index, int num_entradas){
    printf("\n=== ÍNDICE INVERTIDO ===\n");
    printf("Formato: palabra  página:cantidad  página:cantidad ...\n\n");
    for(int i =0; i< num_entradas; i++){
        printf("%s",index[i].palabra);
        for(int j = 0; j < index[i].num_paginas; j++){
            printf(" %d:%d  ", index[i].paginas[j], index[i].contador[j]);
        }
        printf("\n");    
    }
}


int main(int argc, char* argv[]){
    //verificar argumentos 
    if(argc !=3){
        printf("Uso: %s <archivo_libro> <archivo_palabras>\n", argv[0]);
        printf("Ejemplo: %s libro.txt palabras.txt\n", argv[0]);
        return 1;
    }
    
    const char* archivo_libro = argv[1];
    const char* archivo_palabras = argv[2];
    
    //Cargar palabras de busqueda
    if(!abrir_archivo(archivo_palabras)){
        return 1;
    }   
    
    //cargar libro
    char* contenido_libro = leer_libro(archivo_libro);
    if(!contenido_libro){
        return 1;    
    }
    //Dividir el texto en páginas usando \n\n\n como separados
    char** paginas = malloc(MAX_PAGINAS * sizeof(char*));
    if(!paginas){
        printf("Error: no se pudo asignar memoria para las páginas\n");
        free(contenido_libro);
        return 1;    
    }   
    
    int numero_paginas = 0;
    //Buscar la secuencia \n\n\n
    char* posicion_actual = contenido_libro;
    char* inicio_pagina =posicion_actual;

    while(*posicion_actual && numero_paginas < MAX_PAGINAS){
        //Buscar la secuencia \n\n\n
        if(posicion_actual[0] == '\n' && posicion_actual[1] == '\n' && posicion_actual[2] == '\n'){
            size_t longitud_pagina = posicion_actual - inicio_pagina;
            if(longitud_pagina > 0){
                //Crear una copia de la página
                char* contenido_pagina = malloc(longitud_pagina + 1);
                if(!contenido_pagina){
                    printf("Error no se puede asignar memoria para la página %d\n", numero_paginas + 1);
                    //Limpiar memoria asignada 
                    for( int i = 0; i < numero_paginas; i++){
                        free(paginas[i]);                    
                    }
                    free(paginas);
                    free(contenido_libro);
                    return 1;                
                }

                strncpy(contenido_pagina, inicio_pagina, longitud_pagina);
                contenido_pagina[longitud_pagina] = '\0';
                //Limpiar espacios al inicio y final

                char* inicio_limpio = contenido_pagina;
                while(*inicio_limpio == ' ' || *inicio_limpio == '\n' || *inicio_limpio =='\r' || *inicio_limpio =='\t'){
                    inicio_limpio++;                
                }
                if(strlen(inicio_limpio) > 0){
                    paginas[numero_paginas] = strdup(inicio_limpio);
                    if(!paginas[numero_paginas]){
                        printf("Error no se pudo asignar memoria para la pagina %d\n", numero_paginas + 1);
                        free(contenido_pagina);
                        //limpiar memoria ya asignada 
                        for(int i = 0; i < numero_paginas; i++){
                            free(paginas[i]);                        
                        }
                        free(paginas);
                        free(contenido_libro);
                        return 1;
                    }
                    numero_paginas++;
                }
                free(contenido_pagina);
            }
            //Saltar los tres \n
            posicion_actual += 3;
            inicio_pagina = posicion_actual;
       
        } else {
            posicion_actual++;        
        }

    }

    //Procesar la última paginas (despues del ultimo separados)
    if(inicio_pagina < posicion_actual && numero_paginas < MAX_PAGINAS){
        char* inicio_limpio = inicio_pagina;
        while(*inicio_limpio == ' ' || *inicio_limpio == '\n' || *inicio_limpio == '\r' || *inicio_limpio == '\t'){
            inicio_limpio++;
        }

        if(strlen(inicio_limpio) > 0){
            paginas[numero_paginas] = strdup(inicio_limpio);
            if(!paginas[numero_paginas]){
                printf("error no se puede asignar memoria para la ultima pagina \n");
                //limpiar memoria asignada
                for(int i = 0; i < numero_paginas; i++){
                    free(paginas[i]);                
                }
                free(paginas);
                free(contenido_libro);
                return 1;            
            }
            numero_paginas++;
        }   
    }

    if(numero_paginas == 0){
        printf("Error no se encontraron páginas en el libro");
        free(contenido_libro);
        for(int i = 0; i < numero_paginas; i++){
            free(paginas[i]);        
        }
        free(paginas);
        return 1;
    }
    
    if(numero_paginas >= MAX_PAGINAS){
        printf("Advertencia: El libro tiene demasiadas paginas (%d). Se procesaran solo las primeras %d.\n",numero_paginas, MAX_PAGINAS);
        numero_paginas = MAX_PAGINAS;
    }

    printf("Procesando %d paginas con %d palabras de busqueda...\n", numero_paginas, num_palabras);

    //Crear pipes y procesos hijo
    int(*pipes)[2] = malloc(numero_paginas * sizeof(int[2]));
    pid_t* pids = malloc(numero_paginas * sizeof(pid_t));

    if(!pipes || !pids){
        printf("Error no se pudo asignar memoria para pipes o PIDs\n");
        //limpiar memoria
        for(int i = 0; i < numero_paginas; i++){
            free(paginas[i]);
        }
        free(paginas);
        free(contenido_libro);
        if(pipes) free(pipes);
        if(pids) free(pids);
        return 1;    
    }
    for(int i = 0; i< numero_paginas; i++){
        if(pipe(pipes[i]) == -1){
            perror("Error creando pipe");
            exit(1);
        }
        pids[i] = fork();
        if (pids[i] == -1){
            perror("Error en fork");
            exit(1);        
        }
        if(pids[i]== 0){
            //Proceso hijo
            close(pipes[i][0]); //Cerrar lectura
            
            printf("proceso hijo %d procesando pagina %d\n", getpid(), i + 1);
            procesar_pagina(i +1, paginas[i], pipes[i][1]);

            exit(0);        
        } else{
            //proceso padre
            close(pipes[i][1]); //Cerrar escritura        
        }
    
    }

    //Proceso padre recolectar resultados 
    indice* index = malloc(MAX_PALABRAS * sizeof(indice));
    if(!index){
        printf("Error no se pudo asignar memoria para el indice\n");
        //limpiar memoria y esperar procesos hijo
        for(int i = 0; i < numero_paginas; i++){
           close(pipes[i][0]);
            waitpid(pids[i], NULL, 0);     
        }
        free(contenido_libro);
        for(int i = 0; i < numero_paginas; i++){
            free(paginas[i]);        
        }
        free(paginas);
        free(pipes);
        free(pids);
        return 1;
    }
    int num_entradas_indice = 0;
    
    for(int i = 0; i < numero_paginas; i++){
        infoPalabra info_palabras;
    
        //Leer todas las palabras de este proceso hijo
        while(read(pipes[i][0], &info_palabras, sizeof(infoPalabra))>0){
            //Buscar si la palabra ya existe en el indice
            int encontrada = 0;
            for(int j = 0; j< num_entradas_indice; j++){
                if(strcmp(index[j].palabra, info_palabras.palabra)== 0){
                    //Palabra encontrada, agregar informacion de pagina
                    index[j].paginas[index[j].num_paginas] =info_palabras.pagina;
                    index[j].contador[index[j].num_paginas] = info_palabras.contador;
                    index[j].num_paginas++;
                    encontrada = 1;
                    break;
                }            
            }
            if(!encontrada){
                //nueva palabra, agregar al indice
                strcpy(index[num_entradas_indice].palabra, info_palabras.palabra);
                index[num_entradas_indice].paginas[0] = info_palabras.pagina;
                index[num_entradas_indice].contador[0]= info_palabras.contador;
                index[num_entradas_indice].num_paginas =1;
                num_entradas_indice++;
            }
        }
        
        close(pipes[i][0]);

        //Esperar a que termine el proceso hijo
        int estado;
        waitpid(pids[i], &estado, 0);
        printf("proceso hijo para pagina %d terminado\n", i + 1);            
    }
    //Ordenar el indice 
    qsort(index, num_entradas_indice, sizeof(indice), comparar_indice);
    //Imprimir el indice invertido 
    print_indice_invertido(index, num_entradas_indice);
    //limpiar memoria
    free(contenido_libro);
    for(int i = 0; i < numero_paginas; i++){
        free(paginas[i]);
    }
    free(paginas);
    free(pipes);
    free(pids);
    free(index);

    printf("\nProcesamiento completado exitosamente.\n");
    printf("Estadísticas:\n");
    printf("- Páginas procesadas: %d\n", numero_paginas);
    printf("- Palabras buscadas: %d\n", num_palabras);
    printf("- Palabras encontradas: %d\n", num_entradas_indice);

    return 0;
}






















































































