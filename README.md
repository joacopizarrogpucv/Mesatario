# Mesatario Recetario
Proyecto desarrollado en C para la asignatura de Estructura de Datos.

## Integrantes
- Joaquín Pizarro
- Matías Andrade

## Descripción
Mesatario Recetario es una aplicación que permite administrar y consultar recetas de cocina según se pidan. El programa permite cargar recetas desde archivos CSV, buscar recetas por ingredientes, revisar instrucciones paso a paso, calificar recetas, gestionar una despensa personal y recibir recomendaciones automáticas.

## Funcionalidades principales
- Cargar recetas desde `Recetario.csv`.
- Cargar calificaciones desde `Calificaciones.csv`.
- Buscar recetas según ingredientes ingresados por el usuario.
- Mostrar porcentaje de coincidencia e ingredientes faltantes.
- Ver instrucciones paso a paso.
- Calificar recetas.
- Registrar usuario.
- Crear, mostrar y eliminar recetas personales.
- Gestionar despensa.
- Usar Chef Personal según ingredientes disponibles.
- Usar Chef Aleatorio.

## Estructuras de datos utilizadas
- **Lista:** recetas, ingredientes, pasos, despensa, recetas personales y calificaciones.
- **Mapa:** relación de ingrediente-recetas y relación de ID-receta.
- **Pila:** navegación de instrucciones, revista y recetas personales (avanzar/retroceder).

## Archivos necesarios
El programa necesita los siguientes archivos en la misma carpeta:

- `codigo.c`
- `Recetario.csv`
- `Calificaciones.csv`
- Carpeta `tdas/`

## Compilación
Para compilar el programa:

```bash
gcc codigo.c tdas/*.c -o programa
```
## Ejecución
Para ejecutar el programa:

```bash
./programa
```

## Uso general
1. Cargar el recetario desde el menú principal.
2. Registrar un usuario.
3. Buscar recetas por ingredientes o administrar la despensa.
4. Revisar recetas, instrucciones y calificaciones.
5. Utilizar el Chef Personal o el Chef Aleatorio para recibir recomendaciones.

## Contribuciones

### Joaquín Pizarro
- Implementación de la búsqueda por ingredientes.
- Sistema de porcentajes de coincidencia.
- Ordenamiento de resultados.
- Gestión de la despensa.
- Implementación del Chef Personal.
- Documentación y presentación del proyecto.

### Matías Andrade
- Carga de recetas y calificaciones desde archivos CSV.
- Gestión de usuarios.
- Calificación de recetas.
- Recetas personales.
- Navegación por instrucciones mediante pilas.
- Implementación del Chef Aleatorio.

## Notas
- Las recetas personales y la despensa solo se mantienen durante la ejecución del programa.
- Las recetas y calificaciones iniciales se cargan desde los archivos CSV.
- El proyecto fue desarrollado utilizando estructuras de datos dinámicas y TDAs implementados durante la asignatura.