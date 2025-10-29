#include "../micro.h"

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
    /* 0 */ { 1, 3, 5, 6, 7, 8, 9, 10, 11, 20, 13, 0, 14, 17, 18, 14 }, 
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
    /* 19 */{ 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14 },  // Final Op. Relacional
    /* 20 */ { 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19 }, // "=="
  };

  //memset(buffer, 0, sizeof(buffer));
  int car;
  int col;
  int estado = 0;
  int i = 0;
  
  memset(buffer, 0, sizeof(buffer));
  
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
    case 17: // < o >
          buffer[i] = '\0';
      // Si lo que sigue no pertenece al siguiente token, devolverlo
      if (car != EOF && !isspace(car) && car != ';' && car != ')' && car != '(')
        ungetc(car, in);
      return OP_RELACIONAL;
    case 19: // >= o <= o == o !=
    case 20:
      buffer[i] = '\0';
      // No devolver el carácter, porque ya fue consumido por lookahead.
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
  if(e == 0 || e == 1 || e == 3 || e == 11 || e == 15 || e == 17 || e == 18 || e == 20) return 0;
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
  return 15;
}