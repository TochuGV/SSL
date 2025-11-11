# Trabajo Práctico Final - Sintáxis y Semántica de los Lenguajes (2025)

## Compilador del Lenguaje "Micro"

Este repositorio contiene el código fuente del trabajo práctico final de la materia. El proyecto consiste en la extensión de un compilador base para el lenguaje "Micro", añadiendo nuevas funcionalidades sintácticas y semánticas.

El compilador está escrito en C y genera código de tres direcciones como salida.

## Características Implementadas

### Tipos de Datos
- **`int`**: Números enteros
- **`float`**: Números reales de punto flotante
- **`char`**: Caracteres individuales (entre comillas simples, ej: `'a'`)

### Estructuras de Control
- **Condicional `si-entonces-sino`**: Ejecución condicional de bloques de código
- **Bucle `mientras`**: Repetición mientras se cumpla una condición
- **Bucle `repetir-hasta`**: Repetición hasta que se cumpla una condición (ejecuta al menos una vez)

### Operaciones
- **Aritméticas**: suma (`+`), resta (`-`)
- **Relacionales**: `==`, `!=`, `<`, `>`, `<=`, `>=`
- **Asignación**: `:=`
- **Entrada/Salida**: `leer()`, `escribir()`

### Análisis Semántico
- Verificación de tipos en asignaciones y operaciones
- Control de declaración de variables antes de su uso
- Detección de errores de tipo en tiempo de compilación
- Generación de temporales con tipos correctos

## Estructura del Proyecto

```
.
├── src/
│   ├── scanner.c       # Analizador léxico (scanner)
│   └── parser.c        # Analizador sintáctico y semántico
├── main.c              # Punto de entrada del compilador
├── micro.exe           #
├── micro.m             # Programa de ejemplo
└── micro.h             # Declaraciones globales y prototipos
```

## Compilación

Para compilar el proyecto, utiliza el siguiente comando:

```bash
gcc main.c src/scanner.c src/parser.c -o micro
```

## Uso

Para ejecutar el compilador sobre un archivo fuente en lenguaje Micro:

```bash
./micro micro.m
```

El compilador leerá el archivo `micro.m` y generará el código intermedio por salida estándar.
