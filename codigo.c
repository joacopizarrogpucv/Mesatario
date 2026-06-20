#include "tdas/extra.h"
#include "tdas/list.h"
#include "tdas/map.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

typedef struct 
{
  int ID;
  char nombre[30];
  List *ingredientes;
  List *pasos;
  int tiempoPreparacion;
  float calificacionPromedio;
  int cantidadCalificaciones;
  List *usuariosCalificadores;
} Receta;
typedef struct 
{
  char nombre[30];
  List *despensa;
  List *tuRecetario;
  List *calificacionesRealizadas;
} Usuario;
typedef struct 
{
  List *recetas;
  Map *recetasPorIngrediente;
  Map *recetasPorID;
  Usuario usuarioActual;
  List *ingredientesGlobales;
} Recetario;

//funciones generales
void mostrarMenuPrincipal() 
{
  limpiarPantalla();
  puts("========================================");
  puts("               Mesatario");
  puts("========================================");

  puts("1) Cargar Recetario");
  puts("2) Buscar Recetas");
  puts("3) Información");
  puts("4) Tu Recetario");
  puts("5) Recomendación del Chef");
  puts("6) Registrarse");
  puts("7) Cerrar Recetario");
}

int is_equal_str(void *key1, void *key2) 
{
  return strcmp((char *)key1, (char *)key2) == 0;
}

int is_equal_int(void *key1, void *key2) 
{
  return *(int *)key1 == *(int *)key2; // Compara valores enteros directamente
}

//funciones programa
void cargarCalificaciones(Recetario r*){
  FILE *archivo = fopen("Calificaciones.csv", "r");
  if (archivo == NULL) 
  {
    perror("Error al abrir el archivo."); 
    return;
  }

  char **campos;
  campos = leer_linea_csv(archivo, ',');
  while ((campos = leer_linea_csv(archivo, ',')) != NULL) 
  {
    int idReceta = atoi(campos[0]);
    char *nombreUsuario = campos[1];
    float nuevaCalificacion = (float)atof(campos[2]);

    MapPair *par = map_search(r->recetasPorID, &idReceta);
    if (par == NULL){
      printf("Advertencia: No existe una receta con el ID %d.\n", idReceta);
      continue;
    }
    
    Receta *receta = (Receta *)par->value;
    receta->calificacionPromedio = (receta->calificacionPromedio * receta->cantidadCalificaciones + nuevaCalificacion) / (receta->cantidadCalificaciones + 1)
      receta->cantidadCalificaciones++;

    list_pushBack(receta->usuariosCalificadores, strdup(nombreUsuario));
  }
    fclose(archivo);
}

void cargarRecetario(Recetario *r)
{
  r->recetas = list_create();
  r->recetasPorIngrediente = map_create(is_equal_str);
  r->recetasPorID = map_create(is_equal_int);
  r->ingredientesGlobales = list_create();
  FILE *archivo = fopen("Recetario.csv", "r");
  if (archivo == NULL) 
  {
    perror("Error al abrir el archivo."); 
    return;
  }
  char **campos;
  campos = leer_linea_csv(archivo, ','); 
  while ((campos = leer_linea_csv(archivo, ',')) != NULL) 
  {
    Receta *receta = (Receta *)malloc(sizeof(Receta));
    receta->ID = atoi(campos[0]);       
    strcpy(receta->nombre, campos[1]);
    receta->ingredientes = split_string(campos[2], ",");      
    receta->pasos = split_string(campos[3], ",");
    receta->tiempoPreparacion = atoi(campos[4]);
    receta->calificacionPromedio = 0.0f;
    receta->cantidadCalificaciones = 0.0;
    receta->usuariosCalificadores = list_create();
    int *id = malloc(sizeof(int));
    *id = receta->ID;
    map_insert(r->recetasPorID, id, receta);
    list_pushBack(r->recetas, receta);
    
    char *ingrediente = list_first(receta->ingredientes);
    while (ingrediente != NULL) 
    {
      MapPair *key = map_search(r->recetasPorIngrediente, ingrediente);
      if (key == NULL) 
      {
        List *lista_nueva = list_create();
        list_pushBack(lista_nueva, receta);
        map_insert(r->recetasPorIngrediente, ingrediente, lista_nueva);
      } 
      else 
      {
        List *lista_existente = (List *)key->value;
        list_pushBack(lista_existente, receta);
      }
      list_pushBack(r->ingredientesGlobales, ingrediente);
      ingrediente = list_next(receta->ingredientes);
    }
  }
  fclose(archivo);

  cargarCalificaciones(r);
}

void buscarReceta(Recetario *r)
{
  bool encontrado = false;
  limpiarPantalla();
  
  puts("Ingrese el o los ingrediente(s) separados por una coma (,) y en minúsculas");
  puts("Ejemplo: arroz, cebolla, pollo ");
  char linea[50];
  scanf("%[^\n]", linea);
  
  List *lista = split_string(linea, ",");
  
  char *ingrediente = list_first(lista);
  while(ingrediente !=NULL)
  {
    MapPair *key = map_search(r->recetasPorIngrediente, ingrediente);
    if (key == NULL)
    {
      printf("%s no encontrado\n", ingrediente);
    }
    else
    {
      printf("%s encontrado\n", ingrediente);
      encontrado = true;
    }
    ingrediente = list_next(lista);
  }
  if(!encontrado)
  {
    puts("No se encuentraron los ingredientes en el recetario.");
    return;
  }
}

int main() 
{
  char opcion;
  bool x = false;
  Recetario *r = malloc(sizeof(Recetario));
  do 
  {
    mostrarMenuPrincipal();
    printf("Ingrese su opción: ");
    scanf(" %c", &opcion);
    
    switch (opcion)
    {
    case '1':
      if(x) puts("Ya se cargó el recetario");
      else
      {
        cargarRecetario(r);
        x=true;
      }
      break;
    case '2':
      buscarReceta(r);
      break;
    case '3':
      //información
      break;
    case '4':
      //tu recetario
      break;
    case '5':
      //recomendación del chef
      break;
    case '6':
      //Registrar
      break; 
    }
  } while (opcion != '7');
  free(r);
  return 0;
}