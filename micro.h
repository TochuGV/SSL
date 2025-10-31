#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#define NUMESTADOS 24
#define NUMCOLS 17
#define TAMLEX 32+1
#define TAMNOM 20+1

/******************DECLARACIONES GLOBALES*************************/
extern FILE* in;

typedef enum {
  T_ENTERO,
  T_REAL,
  T_CARACTER,
  T_DESCONOCIDO,
} TIPO_DATO;

typedef enum {
  INICIO,
  FIN,
  LEER,
  ESCRIBIR,
  TIPO_INT,
  TIPO_CHAR,
  TIPO_FLOAT,
  ID,
  CONSTANTE_INT,
  CONSTANTE_FLOAT,
  CONSTANTE_CHAR,
  PARENIZQUIERDO,
  PARENDERECHO,
  PUNTOYCOMA,
  COMA,
  ASIGNACION,
  SUMA,
  RESTA,
  FDT,
  ERRORLEXICO,
  SI,
  ENTONCES,
  SINO,
  FIN_SI,
  MIENTRAS,
  HACER,
  FIN_MIENTRAS,
  REPETIR,
  HASTA,
  OP_RELACIONAL,
} TOKEN;

typedef struct {
  char identifi[TAMLEX];
  TOKEN t;
  TIPO_DATO tipo;
} RegTS;

typedef struct {
  TOKEN clase;
  char nombre[TAMLEX];
  int valor_entero;
  float valor_real;
  TIPO_DATO tipo;
} REG_EXPRESION;

extern RegTS TS[1000];
extern char buffer[TAMLEX];
extern TOKEN tokenActual;
extern int flagToken;

/**********************PROTOTIPOS DE FUNCIONES************************/
// Fase léxica (Implementadas en 'scanner.c')
TOKEN scanner();
void procesarFinalToken(int col, int car, FILE* in, char* buffer, int i);
int columna(int c);
int estadoFinal(int e);

// Fase sintáctica (Implementadas en 'parser.c')
void Objetivo(void);
void Programa(void);
void ListaDeclaraciones(void);
void Declaracion(void);
void ListaSentencias(void);
void Sentencia(void);
void ListaIdentificadores(TIPO_DATO tipo_asociado);
void Identificador(REG_EXPRESION* presul);
void ListaExpresiones(void);
void Expresion(REG_EXPRESION* presul);
void Primaria(REG_EXPRESION* presul);
void OperadorAditivo(char* presul);
void SentenciaSi(void);
void SentenciaMientras(void);
void SentenciaRepetirHasta(void);
void Condicion(REG_EXPRESION* presul);
void SentenciaMientras(void);
void SentenciaRepetir(void);
// Rutinas semánticas (Implementadas en 'parser.c')
REG_EXPRESION ProcesarCte(TOKEN clase);
REG_EXPRESION ProcesarId(void);
char* ProcesarOp(void);
void Leer(REG_EXPRESION in);
void Escribir(REG_EXPRESION out);
REG_EXPRESION GenInfijo(REG_EXPRESION e1, char* op, REG_EXPRESION e2);
REG_EXPRESION GenLogico(REG_EXPRESION e1, char* op, REG_EXPRESION e2);
void Comenzar(void);
void Terminar(void);
void Asignar(REG_EXPRESION izq, REG_EXPRESION der);

// Funciones auxiliares (Implementadas en 'parser.c')
void Match(TOKEN t);
TOKEN ProximoToken();
void ErrorLexico();
void ErrorSintactico();
void ErrorSemantico(const char* mensaje);
void Generar(char* co, char* a, char* b, char* c);
char* Extraer(REG_EXPRESION* preg);
int Buscar(char* id, RegTS* TS, TOKEN* t);
void Colocar(char* id, RegTS* TS, TIPO_DATO tipo);
void Chequear(char* s, TIPO_DATO tipo);
char* TipoDatoToString(TIPO_DATO tipo);
char* NuevaEtiqueta(void);
<<<<<<< HEAD
void GenerarEtiqueta(char* e);
REG_EXPRESION GenLogico(REG_EXPRESION e1, char* op, REG_EXPRESION e2);
=======
void GenerarEtiqueta(char* e);
>>>>>>> 1cdb6e1d003f8a9083c26a5ff35667791bd46fc9
