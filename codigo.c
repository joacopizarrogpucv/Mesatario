#include "tdas/extra.h"
#include "tdas/list.h"
#include "tdas/map.h"
#include "tdas/stack.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

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
  Receta *receta;
  int coincidencias;
  float porcentaje;
} ResultadoBusqueda;
typedef struct
{
  int ID;
  float calificacion;
} CalificacionRealizada;
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
  bool estaRegistrado;
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

int is_equal_int(void *key1, void *key2) {
  return *(int *)key1 == *(int *)key2; // Compara valores enteros directamente
}

//funciones programa
void verInstrucciones(Receta *receta){
  if (receta == NULL || list_size(receta->pasos) == 0){
    puts("Esta receta no tiene instrucciones registradas.");
    presioneTeclaParaContinuar();
    return;
  }

  Stack *pasosPendientes = stack_create(NULL);
  Stack *pasosVistos = stack_create(NULL);
  Stack *auxiliar = stack_create(NULL);
  char *paso = list_first(receta->pasos);

  while (paso != NULL){
    stack_push(auxiliar, paso);
    paso = list_next(receta->pasos);
  }
  while (stack_top(auxiliar) != NULL){
    stack_push(pasosPendientes, stack_pop(auxiliar));
  }
  stack_push(pasosVistos, stack_pop(pasosPendientes));
  char opcion;

  do
  {
    limpiarPantalla();
    printf("========================================\n");
    printf("Instrucciones para %s\n", receta->nombre);
    
    printf("Paso %d de %d\n", list_size(pasosVistos), list_size(receta->pasos));
    printf("========================================\n\n");
    printf("%s\n\n", (char *)stack_top(pasosVistos));
    puts("1) Siguiente paso");
    puts("2) Anterior paso");
    puts("0) Volver a la receta");
    printf("Ingrese su opción: ");
    scanf(" %c", &opcion);

    switch (opcion)
      {
      case '1':
        if (stack_top(pasosPendientes) == NULL){
          puts("Ya te encuentras en el último paso.");
          presioneTeclaParaContinuar();
        }
        else{
          stack_push(pasosVistos, stack_pop(pasosPendientes));
        }
        break;
      case '2':
        if (list_size(pasosVistos) <= 1){
          puts("Ya te encuentras en el primer paso.");
          presioneTeclaParaContinuar();
        }
        else{
          stack_push(pasosPendientes, stack_pop(pasosVistos));
        }
        break;
      } 
  } while (opcion != '0');

  //liberación memoria
  stack_clean(pasosPendientes);
  stack_clean(pasosVistos);
  stack_clean(auxiliar);
  free(pasosPendientes);
  free(pasosVistos);
  free(auxiliar);
}

void menuR(Receta *data){
  printf("=======================================================================\n");
  printf("                      %s\n", data->nombre);
  printf("=======================================================================\n");
  printf("ID: %d | Calificación Promedio: %.2f | Calificación Cantidad: %d\n",
        data->ID, data->calificacionPromedio, data->cantidadCalificaciones);
  puts("Lista de Ingredientes:");
  char *aux = list_first(data->ingredientes);
  while(aux != NULL)
  {
    printf(" *%s\n", aux);
    aux = list_next(data->ingredientes);
  }
}

void calificarReceta(Receta *receta, Recetario *r){
  limpiarPantalla();
  if(!r->estaRegistrado)
  {
    puts("Aún no te registras en el recetario (opción 6), debes registrarte antes de calificar recetas.");
    return;
  }
  float calificacion;
  do
  {
    printf("Ingresa una calificacion entre 1 y 10: ");
    scanf("%f", &calificacion);
    if (calificacion < 1 || calificacion > 10)
    {
      puts("La calificacion debe estar entre 1 y 10.");
    }

  } while (calificacion < 1 || calificacion > 10);
  CalificacionRealizada *aux = list_first(r->usuarioActual.calificacionesRealizadas);
  while(aux != NULL)
  {
    if(aux->ID == receta->ID)
    {
      float suma = receta->calificacionPromedio * receta->cantidadCalificaciones;
      suma -= aux->calificacion;
      suma += calificacion;
      aux->calificacion = calificacion;
      receta->calificacionPromedio = suma / receta->cantidadCalificaciones;
      puts("Calificación actualizada.");
      return;
    }
    aux = list_next(r->usuarioActual.calificacionesRealizadas);
  }
  CalificacionRealizada *nueva = (CalificacionRealizada *)malloc(sizeof(CalificacionRealizada));
  if (nueva == NULL){
    puts("No se pudo guardar la calificacion.");
    return;
  }
  nueva->ID = receta->ID;
  nueva->calificacion = calificacion;
  
  float sumatoria = receta->calificacionPromedio * receta->cantidadCalificaciones;
  sumatoria += calificacion;
  receta->cantidadCalificaciones++;
  receta->calificacionPromedio = sumatoria / receta->cantidadCalificaciones;
  list_pushBack(receta->usuariosCalificadores, strdup(r->usuarioActual.nombre));
  list_pushBack(r->usuarioActual.calificacionesRealizadas, nueva);
  puts("Califación agregada correctamente.");
}

void mostrarReceta(Receta *data, Recetario *r){
  char opcion;
  do 
    {
      limpiarPantalla();
      menuR(data); //matias
      puts("1) Ver Instrucciones");
      puts("2) Calificar Receta");
      puts("0) Volver");
      printf("Ingrese su opción: ");
      scanf(" %c", &opcion);

      switch (opcion)
      {
      case '1':
        verInstrucciones(data);
        break;
      case '2':
        calificarReceta(data, r);
        presioneTeclaParaContinuar();
        break;
      }
    } while (opcion != '0');
}

void cargarCalificaciones(Recetario *r){
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
    receta->calificacionPromedio = (receta->calificacionPromedio * receta->cantidadCalificaciones + nuevaCalificacion) / (receta->cantidadCalificaciones + 1);
      receta->cantidadCalificaciones++;

    list_pushBack(receta->usuariosCalificadores, strdup(nombreUsuario));
  }
    fclose(archivo);
}

void cargarRecetario(Recetario *r){
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
    receta->cantidadCalificaciones = 0;
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
        list_pushBack(r->ingredientesGlobales, ingrediente);
      } 
      else 
      {
        List *lista_existente = (List *)key->value;
        list_pushBack(lista_existente, receta);
      }
      ingrediente = list_next(receta->ingredientes);
    }
  }
  fclose(archivo);

  cargarCalificaciones(r);
  printf("Recetario cargado correctamente.\n");
  printf("Recetas cargadas: %d\n", list_size(r->recetas));
}

void liberarResultadosBusqueda(List *resultados){
  ResultadoBusqueda *resultado = list_first(resultados);
  while (resultado != NULL)
  {
    free(resultado);
    resultado = list_next(resultados);
  }
  list_clean(resultados);
  free(resultados);
}

void liberarListaStrings(List *lista){
  if (lista == NULL) return;
  char *texto = list_first(lista);
  while (texto != NULL)
  {
    free(texto);
    texto = list_next(lista);
  }
  list_clean(lista);
  free(lista);
}


//2
ResultadoBusqueda *buscarResultado(List *resultados, Receta *receta){
  ResultadoBusqueda *actual = list_first(resultados);
  while (actual != NULL)
  {
    if (actual->receta->ID == receta->ID){
      return actual;
    }
    actual = list_next(resultados);
  }
  return NULL;
}

bool ingredienteEstaEnLista(List *lista, const char *ingrediente){
  char *actual = list_first(lista);
  while (actual != NULL){
    if (strcmp(actual, ingrediente) == 0){
      return true;
    }
    actual = list_next(lista);
  }
  return false;
}

List *generarResultadosBusqueda(Recetario *r, List *ingredientesBuscados){
  List *resultados = list_create();
  List *ingredientesProcesados = list_create();
  char *ingrediente = list_first(ingredientesBuscados);
  while(ingrediente != NULL){
    if (ingredienteEstaEnLista(ingredientesProcesados, ingrediente)){
      ingrediente = list_next(ingredientesBuscados);
      continue;
    }
    list_pushBack(ingredientesProcesados, ingrediente);
    
    MapPair *parIngrediente = map_search(r->recetasPorIngrediente, ingrediente);
    if (parIngrediente == NULL){
      printf("%s no existe en el recetario.\n", ingrediente);
    }
    else{
      List *recetasConIngrediente = (List *)parIngrediente->value;
      Receta *receta = list_first(recetasConIngrediente);

      while (receta != NULL)
      {
        ResultadoBusqueda *resultado = buscarResultado(resultados, receta);

        if (resultado == NULL){
          resultado = (ResultadoBusqueda *)malloc(sizeof(ResultadoBusqueda));
          resultado->receta = receta;
          resultado->coincidencias = 1;
          resultado->porcentaje = 0.0f;
          list_pushBack(resultados, resultado);
        }
        else{
          resultado->coincidencias++;
        }
        receta = list_next(recetasConIngrediente);
      }
    }
    ingrediente = list_next(ingredientesBuscados);
  }
  
  ResultadoBusqueda *resultado = list_first(resultados);

  while (resultado != NULL){
    int totalIngredientes = list_size(resultado->receta->ingredientes);

    resultado->porcentaje = ((float)resultado->coincidencias * 100.0f) / totalIngredientes;

    resultado = list_next(resultados);
  }
  list_clean(ingredientesProcesados);
  free(ingredientesProcesados);
  return resultados;
}

void ordenarResultados(ResultadoBusqueda **arreglo, int cantidad){
  for (int i = 0; i < cantidad - 1; i++){
    for (int k = 0; k < cantidad - 1 - i; k++){
      bool cambiar = false;
      if (arreglo[k]->porcentaje < arreglo[k + 1]->porcentaje){
        cambiar = true; //mayor porcentaje.
      }
      // mismo porcentaje gana con más coincidencias
      else if (arreglo[k]->porcentaje == arreglo[k + 1]->porcentaje && arreglo[k]->coincidencias < arreglo[k + 1]->coincidencias){
        cambiar = true;
      }
      // si denuevo empate gana mejor calificada
      else if (arreglo[k]->porcentaje == arreglo[k + 1]->porcentaje && arreglo[k]->coincidencias == arreglo[k + 1]->coincidencias &&
               arreglo[k]->receta->calificacionPromedio < arreglo[k + 1]->receta->calificacionPromedio){
        cambiar = true;
      }
      if (cambiar){
        ResultadoBusqueda *aux = arreglo[k];
        arreglo[k] = arreglo[k + 1];
        arreglo[k + 1] = aux;
      }
    }
  }
}

int contarFaltantes(Receta *receta, List *ingredientesBuscados){
  int faltantes = 0;
  char *ingrediente = list_first(receta->ingredientes);
  while (ingrediente != NULL){
    if (!ingredienteEstaEnLista(ingredientesBuscados, ingrediente)){
      faltantes++;
    }
    ingrediente = list_next(receta->ingredientes);
  }
  return faltantes;
}

void mostrarFaltantes(Receta *receta, List *ingredientesBuscados){
  int cantidadFaltantes = contarFaltantes(receta, ingredientesBuscados);
  if (cantidadFaltantes == 0){
    return;
  }
  if (cantidadFaltantes == 1){
    printf("   Falta: ");
  }
  else{
    printf("   Faltan: ");
  }
  char *ingrediente = list_first(receta->ingredientes);
  bool primero = true;
  while (ingrediente != NULL){
    if (!ingredienteEstaEnLista(ingredientesBuscados, ingrediente)){
      if (!primero){
        printf(", ");
      }
      printf("%s", ingrediente);
      primero = false;
    }
    ingrediente = list_next(receta->ingredientes);
  }
  printf("\n");
}

void buscarReceta(Recetario *r){
  limpiarPantalla();
  
  puts("Ingrese el o los ingrediente(s) separados por una coma (,) y en minúsculas");
  puts("Ejemplo: arroz, cebolla, pollo ");
  char linea[50];
  scanf(" %49[^\n]", linea);
  
  List *ingredientesBuscados = split_string(linea, ",");
  List *resultados = generarResultadosBusqueda(r, ingredientesBuscados);
  
  if (list_size(resultados) == 0){
    puts("No se encontraron recetas relacionadas.");
    liberarResultadosBusqueda(resultados);
    liberarListaStrings(ingredientesBuscados);
    return;
  }
  int cantidadResultados = list_size(resultados);
  ResultadoBusqueda **arreglo = malloc(sizeof(ResultadoBusqueda *) * cantidadResultados);
  ResultadoBusqueda *resultado = list_first(resultados);
  for (int i = 0; i < cantidadResultados; i++){
    arreglo[i] = resultado;
    resultado = list_next(resultados);
  }
  
  //MOSTRAR POR PÁGINA Y ORDENADAMENTE (separación para no perderse)
  ordenarResultados(arreglo, cantidadResultados);
  int recetasPorPagina = 5;
  int paginaActual = 0;
  int totalPaginas = (cantidadResultados + recetasPorPagina - 1) / recetasPorPagina;
  char opcion;
  do
  {
    limpiarPantalla();
    puts("========================================");
    puts("          Buscar Receta");
    puts("========================================");
    printf("Pagina %d de %d\n\n", paginaActual + 1, totalPaginas);
    int inicio = paginaActual * recetasPorPagina;
    int fin = inicio + recetasPorPagina;
    if (fin > cantidadResultados){
      fin = cantidadResultados;
    }
    int ultimoPorcentaje = -1;
    for (int i = inicio; i < fin; i++){
      int porcentajeMostrado = (int)(arreglo[i]->porcentaje + 0.5f);
      if (porcentajeMostrado != ultimoPorcentaje){
        printf("Coincidencia %d%%\n\n", porcentajeMostrado);
        ultimoPorcentaje = porcentajeMostrado;
      }
      printf("%d) %s\n", i - inicio + 1, arreglo[i]->receta->nombre);
      mostrarFaltantes(arreglo[i]->receta, ingredientesBuscados);
      puts("");
    }
    puts("");
    if (paginaActual < totalPaginas - 1){
      puts("N) Siguiente pagina");
    }
    if (paginaActual > 0){
      puts("P) Pagina anterior");
    }
    puts("0) Volver al Menú Principal");
    printf("Ingrese una opcion: ");
    scanf(" %c", &opcion);
    if (opcion >= '1' && opcion <= '5'){
      int indiceSeleccionado =
        inicio + (opcion - '1');
      if (indiceSeleccionado < fin){
        limpiarPantalla(); //matias
        mostrarReceta(arreglo[indiceSeleccionado]->receta, r);
      }
      else{
        puts("No existe una receta con ese numero en esta pagina.");
        presioneTeclaParaContinuar();
      }
    }
    else if ((opcion == 'n' || opcion == 'N') && paginaActual < totalPaginas - 1){
      paginaActual++;
    }
    else if ((opcion == 'p' || opcion == 'P') && paginaActual > 0){
      paginaActual--;
    }
    else if (opcion != '0'){
      puts("Opcion inválida.");
      presioneTeclaParaContinuar();
    }

  } while (opcion != '0');

  free(arreglo);
  liberarResultadosBusqueda(resultados);
  liberarListaStrings(ingredientesBuscados);
}

//3
void Revista(List *revista){
  if (revista == NULL || list_size(revista) == 0)
  {
    puts("La revista se encuentra vacia.");
    presioneTeclaParaContinuar();
    return;
  }

  Stack *recetasPendientes = stack_create(NULL);
  Stack *recetasVistas = stack_create(NULL);
  Stack *auxiliar = stack_create(NULL);

  Receta *receta = list_first(revista);
  while (receta != NULL)
  {
    stack_push(auxiliar, receta);
    receta = list_next(revista);
  }
  while (stack_top(auxiliar) != NULL)
  {
    stack_push(recetasPendientes, stack_pop(auxiliar));
  }
  stack_push(recetasVistas, stack_pop(recetasPendientes));

  char opcion;
  do
  {
    limpiarPantalla();
    menuR(stack_top(recetasVistas));
    printf("Página %d de %d\n", list_size(recetasVistas), list_size(revista));
    puts("1) Continuar");
    puts("2) Volver");
    puts("0) Cerrar Revista ");
    printf("Ingrese su opción: ");
    scanf(" %c", &opcion); 

    switch (opcion)
    {
    case '1':
      if (stack_top(recetasPendientes) == NULL)
      {
        puts("Ya te encuentras en la última página.");
        presioneTeclaParaContinuar();
      }
      else
      {
        stack_push(recetasVistas, stack_pop(recetasPendientes));
      }
      break;
    case '2':
      if (list_size(recetasVistas) <= 1)
      {
        puts("Ya te encuentras en la primera página.");
        presioneTeclaParaContinuar();
      }
      else
      {
        stack_push(recetasPendientes, stack_pop(recetasVistas));
      }
      break;
    } 
  } while (opcion != '0');

  stack_clean(recetasPendientes);
  stack_clean(recetasVistas);
  stack_clean(auxiliar);
  free(recetasPendientes);
  free(recetasVistas);
  free(auxiliar);
}

void mostrarMenu3(Recetario *r){
  limpiarPantalla();
  puts("========================================");
  puts("               Información");
  puts("========================================");
  printf("Cantidad total de Recetas: %d\n", list_size(r->recetas));
  printf("Cantidad total de Ingredientes: %d\n", list_size(r->ingredientesGlobales));
  List *lista = list_create();
  int contador =0;
  MapPair  *key = map_first(r->recetasPorIngrediente);
  while(key !=NULL)
  {
    if(contador < list_size(key->value)) 
    {
      contador = list_size(key->value);
      list_clean(lista);
      list_pushFront(lista, key->key);
    }
    else if (contador == list_size(key->value)) list_pushFront(lista, key->key);
    key = map_next(r->recetasPorIngrediente);
  }
  bool x= true;
  printf("Ingrediente(s) más común(es): ");
  for(char *ingrediente = list_first(lista); ingrediente != NULL; ingrediente = list_next(lista))
  {
    if(x) 
    {
      printf("%s", ingrediente);
      x=false;
    }
    else
    {
      printf(", %s", ingrediente);
    }
  }
  printf("\n");
  list_clean(lista);
  free(lista);
}

void informacion(Recetario *r){
  char opcion;
  do 
    {
      mostrarMenu3(r);
      puts("1) Ver todos los Ingredientes");
      puts("2) Ver revista de Recetas");
      puts("0) Volver al Menú Principal");
      printf("Ingrese su opción: ");
      scanf(" %c", &opcion);

      switch (opcion)
      {
      case '1':
        limpiarPantalla();
        puts("Lista de todos los ingredientes del recetario:");
        char *aux = list_first(r->ingredientesGlobales);
        while(aux != NULL)
        {
          printf(" *%s\n", aux);
          aux = list_next(r->ingredientesGlobales);
        }
        presioneTeclaParaContinuar();
        break;
      case '2':
        puts("Abriendo Revista de Recetas...");
        presioneTeclaParaContinuar();
        Revista(r->recetas);
        break;
      }
    } while (opcion != '0');

}

//4
void chefPersonal(Recetario *r){
  limpiarPantalla();

  if (!r->estaRegistrado){
    puts("Primero debes registrarte para usar al Chef Personal.");
    return;
  }
  if (r->recetas == NULL || list_size(r->recetas) == 0){
    puts("Primero debes cargar el recetario.");
    return;
  }
  if (r->usuarioActual.despensa == NULL ||
      list_size(r->usuarioActual.despensa) == 0){
    puts("Tu despensa esta vacia. Agrega ingredientes antes de usar al Chef Personal.");
    return;
  }

  List *resultados = generarResultadosBusqueda(r, r->usuarioActual.despensa);
  if (list_size(resultados) == 0){
    puts("No se encontraron recetas relacionadas con tu despensa.");
    liberarResultadosBusqueda(resultados);
    return;
  }

  int cantidadResultados = list_size(resultados);

  ResultadoBusqueda **arreglo = malloc(sizeof(ResultadoBusqueda *) * cantidadResultados);
  ResultadoBusqueda *resultado = list_first(resultados);
  for (int i = 0; i < cantidadResultados; i++){
    arreglo[i] = resultado;
    resultado = list_next(resultados);
  }
  ordenarResultados(arreglo, cantidadResultados);

  ResultadoBusqueda *mejorResultado = arreglo[0];
  puts("========================================");
  puts("       Recomendacion Chef Personal");
  puts("========================================");
  printf("Recomendación: %s\n", mejorResultado->receta->nombre);
  printf("Coincidencia: %.0f%%\n", mejorResultado->porcentaje);
  
  mostrarFaltantes(mejorResultado->receta, r->usuarioActual.despensa);
  puts("");
  puts("1) Ver receta");
  puts("0) Volver");
  printf("Ingrese una opcion: ");
  char opcion;
  scanf(" %c", &opcion);
  if (opcion == '1'){
    limpiarPantalla();
    mostrarReceta(mejorResultado->receta, r);
  }
  else if (opcion != '0'){
    puts("Opcion invalida.");
    presioneTeclaParaContinuar();
  }

  free(arreglo);
  
  liberarResultadosBusqueda(resultados);
}

void agregarRecetaPersonal(Recetario *r){
  limpiarPantalla();
  puts("========================================");
  puts("        Agregar Receta Personal");
  puts("========================================");
  Receta *receta = malloc(sizeof(Receta));
  if (receta == NULL){
    puts("No se pudo reservar memoria para la receta.");
    return;
  }
  
  char ingredientesTexto[300];
  char pasosTexto[500];
  printf("Nombre de la receta (máximo 30 caracteres): ");
  if (scanf(" %29[^\n]", receta->nombre) != 1){
    puts("Nombre no valido.");
    free(receta);
    return;
  }
  printf("Ingredientes separados por coma (máximo 300 caracteres): ");
  if (scanf(" %299[^\n]", ingredientesTexto) != 1){
    puts("Ingredientes no validos.");
    free(receta);
    return;
  }
  printf("Pasos separados por coma (máximo 500 caracteres): ");
  if (scanf(" %499[^\n]", pasosTexto) != 1){
    puts("Pasos no validos.");
    free(receta);
    return;
  }
  printf("Tiempo de preparacion en minutos: ");
  if (scanf(" %d", &receta->tiempoPreparacion) != 1 || receta->tiempoPreparacion < 0){
    puts("Tiempo de preparacion no valido.");
    free(receta);
    return;
  }

  receta->ID = -1;
  receta->ingredientes = split_string(ingredientesTexto, ",");
  receta->pasos = split_string(pasosTexto, ",");
  receta->calificacionPromedio = 0.0f;
  receta->cantidadCalificaciones = 0;
  receta->usuariosCalificadores = list_create();
  list_pushBack(r->usuarioActual.tuRecetario, receta);
  printf("La receta %s fue agregada a tu recetario.\n", receta->nombre);
}

void mostrarRecetaPersonal(Receta *receta){
  printf("========================================\n");
  printf("%s\n", receta->nombre);
  printf("Tiempo de preparacion: %d minutos\n", receta->tiempoPreparacion);
  printf("========================================\n");
  puts("Ingredientes:");
  char *ingrediente = list_first(receta->ingredientes);
  while (ingrediente != NULL){
    printf(" * %s\n", ingrediente);
    ingrediente = list_next(receta->ingredientes);
  }
}

void verRecetarioPersonal(Recetario *r){
  List *recetario = r->usuarioActual.tuRecetario;
  if (recetario == NULL || list_size(recetario) == 0){
    puts("Aún no hay recetas personales guardadas.");
    presioneTeclaParaContinuar();
    return;
  }

  Stack *pendientes = stack_create(NULL);
  Stack *vistas = stack_create(NULL);
  Stack *auxiliar = stack_create(NULL);
  Receta *receta = list_first(recetario);
  while (receta != NULL){
    stack_push(auxiliar, receta);
    receta = list_next(recetario);
  }
  while (stack_top(auxiliar) != NULL){
    stack_push(pendientes, stack_pop(auxiliar));
  }
  stack_push(vistas, stack_pop(pendientes));

  char opcion;
  do
  {
    limpiarPantalla();
    Receta *actual = stack_top(vistas);
    mostrarRecetaPersonal(actual);
    printf("Receta %d de %d\n\n",
           list_size(vistas),
           list_size(recetario));
    puts("1) Siguiente receta");
    puts("2) Receta anterior");
    puts("3) Ver instrucciones");
    puts("0) Cerrar tu recetario");
    printf("Ingrese su opcion: ");
    scanf(" %c", &opcion);

    switch (opcion)
    {
      case '1':
        if (stack_top(pendientes) == NULL)
        {
          puts("Ya estas en la ultima receta.");
          presioneTeclaParaContinuar();
        }
        else
        {
          stack_push(vistas, stack_pop(pendientes));
        }
        break;
      case '2':
        if (list_size(vistas) <= 1)
        {
          puts("Ya estas en la primera receta.");
          presioneTeclaParaContinuar();
        }
        else
        {
          stack_push(pendientes, stack_pop(vistas));
        }
        break;
      case '3':
        verInstrucciones(actual);
        break;
    }

  } while (opcion != '0');

  stack_clean(pendientes);
  stack_clean(vistas);
  stack_clean(auxiliar);

  free(pendientes);
  free(vistas);
  free(auxiliar);
}

void eliminarRecetaPersonal(Recetario *r){
  List *recetario = r->usuarioActual.tuRecetario;
  if (recetario == NULL || list_size(recetario) == 0){
    puts("No tienes recetas personales para eliminar.");
    return;
  }
  limpiarPantalla();
  puts("========================================");
  puts("        Eliminar Receta Personal");
  puts("========================================");
  int numero = 1;
  Receta *receta = list_first(recetario);
  while (receta != NULL)
  {
    printf("%d) %s\n", numero, receta->nombre);

    numero++;
    receta = list_next(recetario);
  }
  int seleccion;
  printf("Seleccione la receta que desea eliminar (0 para volver): ");
  if (scanf(" %d", &seleccion) != 1){
    puts("Opcion no valida.");
    return;
  }
  if (seleccion == 0) return;
  if (seleccion < 1 || seleccion > list_size(recetario)){
    puts("Numero de receta no valido.");
    return;
  }
  
  receta = list_first(recetario);
  for (int i = 1; i < seleccion; i++){
    receta = list_next(recetario);
  }

  Receta *eliminada = list_popCurrent(recetario);
  if (eliminada == NULL) return;
  printf("La receta %s fue eliminada de tu recetario.\n", eliminada->nombre);
  liberarListaStrings(eliminada->ingredientes);
  liberarListaStrings(eliminada->pasos);
  liberarListaStrings(eliminada->usuariosCalificadores);
  free(eliminada);
}

bool ingredienteEnDespensa(List *despensa, const char *ingrediente){
  char *actual = list_first(despensa);
  while (actual != NULL){
    if (strcmp(actual, ingrediente) == 0){
      return true;
    }
    actual = list_next(despensa);
  }
  return false;
}

void mostrarDespensa(Recetario *r){
  limpiarPantalla();

  puts("========================================");
  printf("      Despensa de %s\n", r->usuarioActual.nombre);
  puts("========================================");

  if (list_size(r->usuarioActual.despensa) == 0)
  {
    puts("Tu despensa esta vacia.");
    return;
  }

  char *ingrediente = list_first(r->usuarioActual.despensa);

  while (ingrediente != NULL)
  {
    printf(" * %s\n", ingrediente);
    ingrediente = list_next(r->usuarioActual.despensa);
  }
}

void agregarIngredienteDespensa(Recetario *r){
  limpiarPantalla();

  char ingrediente[50];

  puts("========================================");
  puts("        Agregar a tu Despensa");
  puts("========================================");

  printf("Ingrese un ingrediente en minusculas: ");

  if (scanf(" %49[^\n]", ingrediente) != 1)
  {
    puts("Ingrediente no valido.");
    return;
  }

  if (ingredienteEnDespensa(r->usuarioActual.despensa, ingrediente))
  {
    puts("Ese ingrediente ya esta en tu despensa.");
    return;
  }

  list_pushBack(r->usuarioActual.despensa, strdup(ingrediente));

  printf("%s fue agregado a tu despensa.\n", ingrediente);
}

void menuDespensa(Recetario *r){
  char opcion;

  do
  {
    limpiarPantalla();
    puts("========================================");
    puts("               Despensa");
    puts("========================================");
    puts("1) Mostrar despensa");
    puts("2) Agregar ingrediente");
    puts("0) Volver a tu recetario");
    printf("Ingrese su opcion: ");
    scanf(" %c", &opcion);

    switch (opcion)
    {
      case '1':
        mostrarDespensa(r);
        presioneTeclaParaContinuar();
        break;

      case '2':
        agregarIngredienteDespensa(r);
        presioneTeclaParaContinuar();
        break;
    }

  } while (opcion != '0');
}

void menuTuRecetario(Recetario *r){
  if (!r->estaRegistrado)
  {
    puts("Primero debes registrarte para usar Tu Recetario.");
    presioneTeclaParaContinuar();
    return;
  }

  char opcion;

  do
  {
    limpiarPantalla();

    puts("========================================");
    printf("       Tu Recetario, %s\n", r->usuarioActual.nombre);
    puts("========================================");
    puts("1) Agregar receta personal");
    puts("2) Eliminar receta personal");
    puts("3) Mostrar tu recetario");
    puts("4) Despensa");
    puts("5) Chef Personal");
    puts("0) Volver al menu principal");
    printf("Ingrese su opcion: ");
    scanf(" %c", &opcion);

    switch (opcion)
    {
      case '1':
        agregarRecetaPersonal(r);
        presioneTeclaParaContinuar();
        break;
      case '2':
        eliminarRecetaPersonal(r);
        presioneTeclaParaContinuar();
        break;
      case '3':
        verRecetarioPersonal(r);
        break;
      case '4':
        menuDespensa(r);
        break;
      case '5':
        chefPersonal(r);
        presioneTeclaParaContinuar();
        break;
    }
  } while (opcion != '0');
}

//5
void recomendacionChef(Recetario *r){
  limpiarPantalla();
  
  int cantidad = list_size(r->recetas);
  if (cantidad == 0){
    puts("El Chef Matías dice que no hay recetas cargadas para recomendar.");
    return;
  }
  int num = rand() % cantidad;
  
  Receta *aux = list_first(r->recetas);
  for (int i = 0; i < num; i++) 
  {
    aux = list_next(r->recetas);
  }
  puts("El Chef Matías te recomienda esta receta:");
  mostrarReceta(aux, r);
}

//6
void registrarUsuario(Recetario *r){
  limpiarPantalla();

  if (r->estaRegistrado){
    printf("Ya existe un usuario registrado: %s\n", r->usuarioActual.nombre);
    puts("No se puede registrar otro usuario durante esta ejecución.");
    return;
  }

  puts("========================================");
  puts("          Registro de Usuario");
  puts("========================================");
  printf("Ingrese su nombre: ");
  if (scanf(" %29[^\n]", r->usuarioActual.nombre) != 1){
    puts("No se pudo registrar el usuario.");
    return;
  }

  r->usuarioActual.despensa = list_create();
  r->usuarioActual.tuRecetario = list_create();
  r->usuarioActual.calificacionesRealizadas = list_create();
  r->estaRegistrado = true;
  printf("\nEl Usuario %s se ha registrado correctamente.\n",
         r->usuarioActual.nombre);
  puts("Ahora puede usar su despensa, recetario personal y calificaciones.");
}

int main() 
{
  srand(time(NULL));
  char opcion;
  bool x = false;
  Recetario *r = calloc(1, sizeof(Recetario));
  do 
  {
    mostrarMenuPrincipal();
    printf("Ingrese su opción: ");
    scanf(" %c", &opcion);
    
    switch (opcion)
    {
    case '1':
      if(x) puts("Ya se cargó el recetario.");
      else {
        cargarRecetario(r);
        x=true;
      }
      presioneTeclaParaContinuar();
      break;
    case '2':
      if (!x){
        puts("Primero se debe cargar el recetario.");
      }
      else {
        buscarReceta(r);
      }
      presioneTeclaParaContinuar();
      break;
    case '3':
      if(!x) puts("Primero se debe cargar el recetario.");
      else
      {
        informacion(r);
      }
      presioneTeclaParaContinuar();

      break;
    case '4':
      menuTuRecetario(r);
      break;
    case '5':
      if(!x) puts("Primero debes cargar el recetario.");
      else
      {
        recomendacionChef(r);
      }
      presioneTeclaParaContinuar();
      break;
    case '6':
      registrarUsuario(r);
      presioneTeclaParaContinuar();
      break; 
    }
  } while (opcion != '7');
  free(r);
  return 0;
}