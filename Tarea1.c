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
    int paginas;
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
int num_palabras = 0

// Función para leer las palabras desde archivo
int abrir_archivo(const char* nomarchivo){
    FILE* archivo = fopen(nomarchivo, "r");
    if (!archivo){
        printf("ERROR al abrir archivo de palabras '%s'\n", nomarchivo); 
        return 0; 
    }
    char linea[MAX_LARGO_PALABRA];
    num_palabras= 0;
   
    while (fgets(linea, sizeof(linea), archivo) && num_palabras < MAX_PALABRAS){
        //Eliminar salto de línea
        linea[strcspn(linea, "\n\r")]= '\0';
        //Convertir a minúsculas 
        for(int i=0; linea[i]; i++){
            linea[i]= tolower(linea[i]): 
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
       printf("Error el archivo es demasiado grande (%ld bytes). Máximo: %d bytes\n", file_size, MAX_LARGO_TEXTO);
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
    printf("SE PUDO LEER EL LIRBO")
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

void procesar_pagina(int num_pagina, conts char* texto_pagina, int escritura){
    infoPalabras contador_palabras[MAX_PALABRAS];

    //inicializar contadores para todas las palabras de busqueda
    for (int i= 0; i < num_palabras; i++){
        strcpy(contador_palabras[i].palabra, busca_palabras[i]);        
    }   

}
