#include "../micro.h";

FILE* in;
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

char buffer[TAMLEX];
TOKEN tokenActual;
int flagToken = 0;

TOKEN scanner(){
  int tabla[NUMESTADOS][NUMCOLS] = { 
    /* 0 */   { 1, 3, 5, 6, 7, 8, 9, 10, 11, 20, 13, 0, 14, 17, 18, 21, 14 },
    /* 1 */   { 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 14 },
    /* 2 */   { 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14 },
    /* 3 */   { 4, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 15, 4, 4, 4, 14 },
    /* 4 */   { 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14 },
    /* 5 */   { 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14 },
    /* 6 */   { 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14 },
    /* 7 */   { 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14 },
    /* 8 */   { 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14 },
    /* 9 */   { 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14 },
    /* 10 */  { 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14 },
    /* 11 */  { 14, 14, 14, 14, 14, 14, 14, 14, 14, 12, 14, 14, 14, 14, 14, 14, 14 },
    /* 12 */  { 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14 },
    /* 13 */  { 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14 },
    /* 14 */  { 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14 },
    /* 15 */  { 16, 15, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 14 },
    /* 16 */  { 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14 },
    /* 17 */  { 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 14 },
    /* 18 */  { 14, 14, 14, 14, 14, 14, 14, 14, 14, 19, 14, 14, 14, 14, 14, 14, 14 },
    /* 19 */  { 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14 },
    /* 20 */  { 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 14 },
    /* 21 */  { 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22 },
    /* 22 */  { 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 23, 22 },
    /* 23 */  { 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14 },
  };

  int car;
  int col;
  int estado = 0;
  int i = 0;

  memset(buffer, 0, sizeof(buffer));
  
  do {
    car = fgetc(in);
    col = columna(car);
    estado = tabla[estado][col];
    if(col != 11 || estado == 22){
      buffer[i] = car;
      i++;
    };
  } while (!estadoFinal(estado) && !(estado == 14));

  buffer[i] = '\0';

  switch(estado){
    case 2: 
      procesarFinalToken(col, car, in, buffer, i);
      return ID;
    case 4:
      procesarFinalToken(col, car, in, buffer, i);
      return CONSTANTE_INT;
    case 16:
      procesarFinalToken(col, car, in, buffer, i);
      return CONSTANTE_FLOAT;
    case 17:
      buffer[i] = '\0';
      // Si lo que sigue no pertenece al siguiente token, devolverlo
      if (car != EOF && !isspace(car) && car != ';' && car != ')' && car != '(') ungetc(car, in);
      return OP_RELACIONAL;
    case 19:
    case 20:
      buffer[i] = '\0';
      // No devolver el carácter, porque ya fue consumido por lookahead.
      return OP_RELACIONAL;
    case 23: 
      if (strlen(buffer) == 3 && buffer[0]=='\'' && buffer[2]=='\'') {
        char valor = (strlen(buffer) == 3) ? buffer[1] : '\0';
        buffer[0] = valor;
        buffer[1] = '\0';
      } else {
        return ERRORLEXICO;
      };
      return CONSTANTE_CHAR;
    case 5: return SUMA;
    case 6: return RESTA;
    case 7: return PARENIZQUIERDO;
    case 8: return PARENDERECHO;
    case 9: return COMA;
    case 10: return PUNTOYCOMA;
    case 12: return ASIGNACION;
    case 13: return FDT;
    case 14: return ERRORLEXICO;
    case 18: return ERRORLEXICO; // Si se queda en 18 es porque leyó '!' y no '='.
  };
  return 0;
};

void procesarFinalToken(int col, int car, FILE* in, char* buffer, int i){
  if (col != 11) {
    ungetc(car, in);
    buffer[i-1] = '\0';
  };
};

int estadoFinal(int e){
  if(e == 0 || e == 1 || e == 3 || e == 11 || e == 15 || e == 17 || e == 18 || e == 20 || e == 21 || e == 22) return 0;
  return 1;
};

int columna(int c){
  if (isalpha(c) || c == '_') return 0;
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
  if (c == '\'') return 15;
  return 16;
};