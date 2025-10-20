#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define NUMESTADOS 20
#define NUMCOLS 16
#define TAMLEX 32+1
#define TAMNOM 20+1

/******************Declaraciones Globales*************************/
FILE* in;

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
  TOKEN t; /* t=0, 1, 2, 3 Palabra Reservada, t=ID=4 Identificador */
  TIPO_DATO tipo;
} RegTS;

RegTS TS[1000] = {
  {"inicio", INICIO, T_DESCONOCIDO},
  {"fin", FIN, T_DESCONOCIDO},
  {"leer", LEER, T_DESCONOCIDO},
  {"escribir", ESCRIBIR, T_DESCONOCIDO},
  {"int", TIPO_INT, T_ENTERO},
  {"char", TIPO_CHAR, T_CARACTER},
  {"float", TIPO_FLOAT, T_REAL},
  {"si", SI, T_DESCONOCIDO},
  {"entonces", ENTONCES, T_DESCONOCIDO},
  {"sino", SINO, T_DESCONOCIDO},
  {"fin_si", FIN_SI, T_DESCONOCIDO},
  {"mientras", MIENTRAS, T_DESCONOCIDO},
  {"hacer", HACER, T_DESCONOCIDO},
  {"fin_mientras", FIN_MIENTRAS, T_DESCONOCIDO},
  {"repetir", REPETIR, T_DESCONOCIDO},
  {"hasta", HASTA, T_DESCONOCIDO},
  {"$", 99, T_DESCONOCIDO}
};

typedef struct {
  TOKEN clase;
  char nombre[TAMLEX];
  int valor_entero;
  float valor_real;
  TIPO_DATO tipo;
} REG_EXPRESION;

char buffer[TAMLEX];
TOKEN tokenActual;
int flagToken = 0;

/**********************Prototipos de Funciones************************/
TOKEN scanner();
int columna(int c);
int estadoFinal(int e);
void Objetivo(void);
void Programa(void);
void ListaSentencias(void);
void Sentencia(void);
void ListaIdentificadores(void);
void Identificador(REG_EXPRESION * presul);
void ListaExpresiones(void);
void Expresion(REG_EXPRESION * presul);
void Primaria(REG_EXPRESION * presul);
void OperadorAditivo(char * presul);
REG_EXPRESION ProcesarCte(void);
REG_EXPRESION ProcesarId(void);
char * ProcesarOp(void);
void Leer(REG_EXPRESION in);
void Escribir(REG_EXPRESION out);
REG_EXPRESION GenInfijo(REG_EXPRESION e1, char * op, REG_EXPRESION e2);
void Match(TOKEN t);
TOKEN ProximoToken();
void ErrorLexico();
void ErrorSintactico();
void Generar(char * co, char * a, char * b, char * c);
char * Extraer(REG_EXPRESION * preg);
int Buscar(char * id, RegTS * TS, TOKEN * t);
void Colocar(char * id, RegTS * TS);
void Chequear(char * s);
void Comenzar(void);
void Terminar(void);
void Asignar(REG_EXPRESION izq, REG_EXPRESION der);

// Función auxiliar

char* TipoDatoToString(TIPO_DATO tipo){
  if (tipo == T_REAL) return "Real";
  if (tipo == T_ENTERO) return "Entero";
  if (tipo == T_CARACTER) return "Caracter";
  return "Desconocido";
}

/***************************Programa Principal************************/
int main(int argc, char* argv[]){
  TOKEN tok;
  char nomArchi[TAMNOM];
  int l;

  /***************************Se abre el Archivo Fuente******************/
  if (argc == 1) {
    printf("Debe ingresar el nombre del archivo fuente (en lenguaje Micro) en la linea de comandos\n");
    return -1;
  };
  
  if (argc != 2) {
    printf("Numero incorrecto de argumentos\n");
    return -1;
  };

  strcpy(nomArchi, argv[1]);
  l = strlen(nomArchi);

  if (l > TAMNOM) {
    printf("Nombre incorrecto del Archivo Fuente\n");
    return -1;
  };
  if (nomArchi[l-1] != 'm' || nomArchi[l-2] != '.') {
    printf("Nombre incorrecto del Archivo Fuente\n");
    return -1;
  };
  if ((in = fopen(nomArchi, "r")) == NULL) {
    printf("No se pudo abrir archivo fuente\n");
    return -1;
  };

  /*************************Inicio Compilacion***************************/
  Objetivo();
  
  /**************************Se cierra el Archivo Fuente******************/
  fclose(in);
  return 0;
}

/**********Procedimientos de Analisis Sintactico (PAS) *****************/
void Objetivo(void){
/* <objetivo> -> <programa> FDT #terminar */
Programa();
Match(FDT);
Terminar();
}

void Programa(void){
/* <programa> -> #comenzar INICIO <listaSentencias> FIN */
Comenzar();// invocacion a las rutinas semanticas, en la gramatica se coloca con #
Match(INICIO);
  //ListaDeclaraciones();
ListaSentencias();
Match(FIN);
}

void ListaSentencias(void){
/* <listaSentencias> -> <sentencia> {<sentencia>} */
Sentencia();
  while(1){
    switch(ProximoToken()){
      case ID:
      case LEER:
      case ESCRIBIR:
Sentencia();
break;
      default: 
        return;
}
}
}

void Sentencia(void){
TOKEN tok = ProximoToken();
REG_EXPRESION izq, der;
  switch(tok){
    case ID:
      /* <sentencia> -> ID := <expresion> #asignar ; (rutina semantica)*/
Identificador(&izq);
Match(ASIGNACION);
Expresion(&der);
Asignar(izq, der);
Match(PUNTOYCOMA);
break;
    case LEER:
      /* <sentencia> -> LEER ( <listaIdentificadores> ) */
Match(LEER);
Match(PARENIZQUIERDO);
ListaIdentificadores();
Match(PARENDERECHO);
Match(PUNTOYCOMA);
break;
    case ESCRIBIR:
      /* <sentencia> -> ESCRIBIR ( <listaExpresiones> ) */
Match(ESCRIBIR);
Match(PARENIZQUIERDO);
ListaExpresiones();
Match(PARENDERECHO);
Match(PUNTOYCOMA);
break;
    default:
      return;
}
}

void ListaIdentificadores(void){
/* <listaIdentificadores> -> <identificador> #leer_id {COMA <identificador> #leer_id} */
TOKEN t;
REG_EXPRESION reg;
Identificador(&reg);
Leer(reg);
  for(t = ProximoToken(); t == COMA; t = ProximoToken()){
  Match(COMA);
Identificador(&reg);
Leer(reg);
}
}

void Identificador(REG_EXPRESION * presul){
/* <identificador> -> ID #procesar_id */
Match(ID);
*presul = ProcesarId();
}

void ListaExpresiones(void){
/* <listaExpresiones> -> <expresion> #escribir_exp {COMA <expresion> #escribir_exp} */
TOKEN t;
REG_EXPRESION reg;
Expresion(&reg);
Escribir(reg);
  for(t = ProximoToken(); t == COMA; t = ProximoToken()){
Match(COMA);
Expresion(&reg);
Escribir(reg);
}
}

void Expresion(REG_EXPRESION * presul){
/* <expresion> -> <primaria> { <operadorAditivo> <primaria> #gen_infijo } */
REG_EXPRESION operandoIzq, operandoDer;
char op[TAMLEX];
TOKEN t;
Primaria(&operandoIzq);
  for(t = ProximoToken(); t == SUMA || t == RESTA; t = ProximoToken()){
OperadorAditivo(op);
Primaria(&operandoDer);
operandoIzq = GenInfijo(operandoIzq, op, operandoDer);
}
*presul = operandoIzq;
}

void Primaria(REG_EXPRESION * presul){
TOKEN tok = ProximoToken();
  switch(tok){
    case ID:
      /* <primaria> -> <identificador> */
Identificador(presul);
break;
    case CONSTANTE_INT:
      /* <primaria> -> CONSTANTE #procesar_cte */
      Match(CONSTANTE_INT);
      *presul = ProcesarCte(CONSTANTE_INT);
      break;
    case CONSTANTE_FLOAT:
      Match(CONSTANTE_FLOAT);
      *presul = ProcesarCte(CONSTANTE_FLOAT);
break;
    case PARENIZQUIERDO:
      /* <primaria> -> PARENIZQUIERDO <expresion> PARENDERECHO */
Match(PARENIZQUIERDO);
Expresion(presul);
Match(PARENDERECHO);
break;
    default:
      return;
}
}

void OperadorAditivo(char * presul){
/* <operadorAditivo> -> SUMA #procesar_op | RESTA #procesar_op */
TOKEN t = ProximoToken();
  if (t == SUMA || t == RESTA){
Match(t);
strcpy(presul, ProcesarOp());
}
  else ErrorSintactico(t);
}
/**********************Rutinas Semanticas******************************/
REG_EXPRESION ProcesarCte(void)
{
/* Convierte cadena que representa numero a numero entero y construye un registro semantico */
REG_EXPRESION reg;
reg.clase = CONSTANTE;
strcpy(reg.nombre, buffer);
sscanf(buffer, "%d", &reg.valor);
return reg;
}

REG_EXPRESION ProcesarCte(TOKEN clase){
  REG_EXPRESION reg;
  reg.clase = clase;
  strcpy(reg.nombre, buffer);

  if (clase == CONSTANTE_INT) {
    reg.tipo = T_ENTERO;
    sscanf(buffer, "%d", &reg.valor_entero);
    // reg.valor_real = (float) reg.valor_entero;
  } else if (clase == CONSTANTE_FLOAT) {
    reg.tipo = T_REAL;
    sscanf(buffer, "%f", &reg.valor_real);
    // reg.valor_entero = (int) reg.valor_real;
  }
  return reg;
};

REG_EXPRESION ProcesarId(void){
/* Declara ID y construye el correspondiente registro semantico */
REG_EXPRESION reg;
  Chequear(buffer,reg.tipo);
reg.clase = ID;
strcpy(reg.nombre, buffer);
return reg;
}

char * ProcesarOp(void){
/* Declara OP y construye el correspondiente registro semantico */
return buffer;
}

void Leer(REG_EXPRESION in){
/* Genera la instruccion para leer */
  Generar("Read", in.nombre, TipoDatoToString(in.tipo), "");
}

void Escribir(REG_EXPRESION out){
/* Genera la instruccion para escribir */
  Generar("Write", Extraer(&out), TipoDatoToString(out.tipo), "");
}

REG_EXPRESION GenInfijo(REG_EXPRESION e1, char * op, REG_EXPRESION e2){
  /* Genera la instruccion para una operacion infija y construye un registro semantico con el resultado */
REG_EXPRESION reg;
static unsigned int numTemp = 1;
char cadTemp[TAMLEX] ="Temp&";
char cadNum[TAMLEX];
char cadOp[TAMLEX];
  
  TIPO_DATO tipoResultado = (e1.tipo == T_REAL || e2.tipo == T_REAL) ? T_REAL : T_ENTERO;

  if(op[0] == '-') strcpy(cadOp, "Restar");
  if(op[0] == '+') strcpy(cadOp, "Sumar");
  
sprintf(cadNum, "%d", numTemp);
numTemp++;
strcat(cadTemp, cadNum);
  
  if(e1.clase == ID) Chequear(Extraer(&e1), e1.tipo);
  if(e2.clase == ID) Chequear(Extraer(&e2), e2.tipo);
  
  Chequear(cadTemp, tipoResultado);
Generar(cadOp, Extraer(&e1), Extraer(&e2), cadTemp);
strcpy(reg.nombre, cadTemp);
  reg.tipo = tipoResultado;
return reg;
}
/***************Funciones Auxiliares**********************************/
void Match(TOKEN t){
  if(!(t == ProximoToken())) ErrorSintactico();
flagToken = 0;
}

TOKEN ProximoToken(){
  if(!flagToken){
tokenActual = scanner();
    if(tokenActual == ERRORLEXICO) ErrorLexico();
flagToken = 1;
    if(tokenActual == ID){
Buscar(buffer, TS, &tokenActual);
}
}
return tokenActual;
}

void ErrorLexico(){
  printf("Error Lexico\n");
};

void ErrorSintactico(){
  printf("Error Sintactico\n");
};

void Generar(char* co, char* a, char* b, char* c){
  /* Produce la salida de la instruccion para la MV por stdout */
  printf("%s %s%c%s%c%s\n", co, a, ',', b, ',', c);
};

char* Extraer(REG_EXPRESION* preg){
  /* Retorna la cadena del registro semantico */
  return preg->nombre;
};

int Buscar(char * id, RegTS * TS, TOKEN * t){
  /* Determina si un identificador esta en la TS */
  int i = 0;
  while (strcmp("$", TS[i].identifi)) {
  if (!strcmp(id, TS[i].identifi)) {
    *t = TS[i].t;
    return 1;
  };
    i++;
  };
  return 0;
};

void Colocar(char* id, RegTS* TS, TIPO_DATO tipo){
  /* Agrega un identificador a la TS */
  int i = 16;
  while (strcmp("$", TS[i].identifi)) i++;
  if (i < 999) {
    strcpy(TS[i].identifi, id);
    TS[i].t = ID;
    TS[i].tipo = tipo;
    strcpy(TS[++i].identifi, "$");
  };
};

void Chequear(char* s, TIPO_DATO tipo){
  /* Si la cadena No esta en la Tabla de Simbolos la agrega,
  y si es el nombre de una variable genera la instruccion */
  TOKEN t;
  if(!Buscar(s, TS, &t)){
    Colocar(s, TS, tipo);
    Generar("Declara", s, TipoDatoToString(tipo), "");
  };
};

void Comenzar(void){
  /* Inicializaciones Semanticas */
};

void Terminar(void){
  /* Genera la instruccion para terminar la ejecucion del programa */
  Generar("Detiene", "", "", "");
};

void Asignar(REG_EXPRESION izq, REG_EXPRESION der){
  /* Genera la instruccion para la asignacion */
  Chequear(izq.nombre, der.tipo);
  Generar("Almacena", Extraer(&der), izq.nombre, "");
};

/**************************Scanner************************************/
TOKEN scanner(){
  /*
  * MAPEO COMPLETO DE ESTADOS DEL ANALIZADOR LÉXICO (NUMESTADOS = 20)
  * * --- ESTADOS BASE Y DE PALABRAS CLAVE (0-10) ---
  * Estado 0: --> INICIAL / REPOSO (Punto de partida del scanner).
  * Estado 1: --> ID EN PROGRESO (Leída la primera letra o dígito: 'i', 'a1').
  * Estado 2: --> FINAL ID/KEYWORD (Devuelve ID, INICIO, SI, FLOAT, etc.).
  * Estado 3: --> CONSTANTE INT EN PROGRESO (Leído el primer dígito: '1').
  * Estado 4: --> FINAL CONSTANTE INT (Devuelve CONSTANTE_INT).
  * * Estado 5: --> FINAL SUMA (+)
  * Estado 6: --> FINAL RESTA (-)
  * Estado 7: --> FINAL PARENIZQUIERDO
  * Estado 8: --> FINAL PARENDERECHO
  * Estado 9: --> FINAL COMA
  * Estado 10: -> FINAL PUNTOYCOMA
  * * --- ESTADOS DE LOOKAHEAD Y ERROR (11-14) ---
  * Estado 11: -> LOOKAHEAD ASIGNACION (Visto ':'). Esperando '='.
  * Estado 12: -> FINAL ASIGNACION (Devuelve ASIGNACION :=).
  * Estado 13: -> FINAL FDT (Fin de Archivo).
  * Estado 14: -> ERROR SUMIDERO (ERRORLEXICO: Patrón inválido o carácter desconocido).
  * * --- ESTADOS DE FLOTANTES Y RELACIONALES (15-19) ---
  * Estado 15: -> LOOKAHEAD CONSTANTE FLOAT (Visto '12.'). Esperando dígitos decimales.
  * Estado 16: -> FINAL CONSTANTE FLOAT (Devuelve CONSTANTE_FLOAT).
  * Estado 17: -> FINAL/LOOKAHEAD OP. RELACIONAL SIMPLE (Visto '=', '<', '>'). 
  * Devuelve OP_RELACIONAL si no sigue otro símbolo.
  * Estado 18: -> LOOKAHEAD OP. NEGACIÓN (Visto '!'). Esperando '=' para '!='.
  * Estado 19: -> FINAL OP. RELACIONAL DOBLE (Devuelve OP_RELACIONAL: <=, >=, !=).
  */

  /*
  * MAPEO DE COLUMNAS DE ENTRADA (NUMCOLS = 16)
  * Col 0: --> Letra (a-z, A-Z)
  * Col 1: --> Dígito (0-9)
  * Col 2: --> Signo '+'
  * Col 3: --> Signo '-'
  * Col 4: --> Signo '('
  * Col 5: --> Signo ')'
  * Col 6: --> Signo ','
  * Col 7: --> Signo ';'
  * Col 8: --> Signo ':'
  * Col 9: --> Signo '=' (Utilizado para relacionales y para completar ':=')
  * Col 10: -> EOF (End of File / Fin de Texto)
  * Col 11: -> Espacio en blanco (space, tab, newline)
  * Col 12: -> Signo '.' (Punto Decimal)
  * Col 13: -> Operadores Relacionales Simples ('<' o '>')
  * Col 14: -> Signo '!' (Lookahead de Operador '!=')
  * Col 15: -> Otro Carácter (Cualquier cosa no mapeada)
  */

  int tabla[NUMESTADOS][NUMCOLS] = { 
    /* 0 */ { 1, 3, 5, 6, 7, 8, 9, 10, 11, 17, 13, 0, 14, 17, 18, 14 }, 
    /* 1 */ { 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2 },       // ID en progreso
    /* 2 */ { 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14 }, 
    /* 3 */ { 4, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 15, 4, 4, 4 },      // INT: en '.' va a 15
    /* 4 */ { 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14 }, 
    /* 5 */ { 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14 }, 
    /* 6 */ { 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14 }, 
    /* 7 */ { 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14 }, 
    /* 8 */ { 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14 }, 
    /* 9 */ { 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14 }, 
    /* 10 */{ 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14 }, 
    /* 11 */{ 14, 14, 14, 14, 14, 14, 14, 14, 14, 12, 14, 14, 14, 14, 14, 14 }, // Lookahead para ASIGNACION (solo acepta '=')
    /* 12 */{ 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14 }, 
    /* 13 */{ 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14 }, 
    /* 14 */{ 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14 }, 
    
    // ESTADOS NUEVOS (Flotantes)
    /* 15 */{ 16, 15, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16 }, // En DIGITO (1) se queda en 15.
    /* 16 */{ 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14 }, 
    
    // ESTADOS NUEVOS (Operadores Relacionales)
    /* 17 */{ 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19 }, // Op. Relacional: en '=' va a 19 (ej: <=)
    /* 18 */{ 14, 14, 14, 14, 14, 14, 14, 14, 14, 19, 14, 14, 14, 14, 14, 14 }, // Lookahead '!' solo acepta '=' para ir a 19 (ej: !=)
    /* 19 */{ 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14 }  // Final Op. Relacional
  };

  int car;
  int col;
  int estado = 0;
  int i = 0;

  do {
    car = fgetc(in);
    col = columna(car);
    estado = tabla[estado][col];
    if(col != 11){
      buffer[i] = car;
      i++;
    };
  } while(!estadoFinal(estado) && !(estado == 14));

  buffer[i] = '\0';

  switch(estado){
    case 2: 
      if (col != 11) {
        ungetc(car, in);
        buffer[i-1] = '\0';
      };
      return ID;
    case 4:
      if (col != 11) {
        ungetc(car, in);
        buffer[i-1] = '\0';
      };
      return CONSTANTE_INT;
    case 16:
      if (col != 11) {
        ungetc(car, in);
        buffer[i-1] = '\0';
      };
      return CONSTANTE_FLOAT;
    case 17:
    case 19:
      if (col != 11) {
        ungetc(car, in);
        buffer[i-1] = '\0';
      };
      return OP_RELACIONAL;
    case 5: return SUMA;
    case 6: return RESTA;
    case 7: return PARENIZQUIERDO;
    case 8: return PARENDERECHO;
    case 9: return COMA;
    case 10: return PUNTOYCOMA;
    case 12: return ASIGNACION;
    case 13: return FDT;
    case 14: return ERRORLEXICO;
    case 18: return ERRORLEXICO; // Si se queda en 18 es porque vio '!' y no '='.
  };
  return 0;
};

int estadoFinal(int e){
  if(e == 0 || e == 1 || e == 3 || e == 11 || e == 14 || e == 15 || e == 18) return 0;
  return 1;
};

int columna(int c){
  if (isalpha(c)) return 0;
  if (isdigit(c)) return 1;
  if (c == '+') return 2;
  if (c == '-') return 3;
  if (c == '(') return 4;
  if (c == ')') return 5;
  if (c == ',') return 6;
  if (c == ';') return 7;
  if (c == ':') return 8;
  if (c == '=') return 9;
  if (c == EOF) return 10;
  if (isspace(c)) return 11;
  if (c == '.') return 12;
  if (c == '<' || c == '>') return 13;
  if (c == '!') return 14;
  return 15;
}

/*************Fin Scanner**********************************************/