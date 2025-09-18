/*Autor: Matias Muñoz Catalán. 
  Profesora: Victoria Flores.
  Curso: Sistenas Operativos.
  Descripción: Tarea busca implementar un programa en C que procesa un libro en formato de texto plano y construye un indice invertido utilizando procesamiento paralelo con la funcón fork() y comunicación entre procesos mediante pipe() */

/// Se utilizan las librerias vistas en clases (presentación "Procesos"). 
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

