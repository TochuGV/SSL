#include "../micro.h"

/***********PROCEDIMIENTOS DE ANÁLISIS SINTÁCTICO (PAS)******************/
void Objetivo(void){
  /* <objetivo> -> <programa> FDT #terminar */
  Programa();
  Match(FDT);
  Terminar();
};

void Programa(void){
  /* <programa> -> #comenzar INICIO <listaSentencias> FIN */
  Comenzar();// invocacion a las rutinas semanticas, en la gramatica se coloca con #
  Match(INICIO);
  ListaDeclaraciones();
  ListaSentencias();
  Match(FIN);
};

void ListaDeclaraciones(void){
  /* <listaDeclaraciones> -> { <Declaracion> } */
  while(ProximoToken() == TIPO_INT || ProximoToken() == TIPO_CHAR || ProximoToken() == TIPO_FLOAT){
    Declaracion();
  };
};

void Declaracion(void){
  /* <declaracion> -> <Tipo> <ListaIdentificadores> ; */
  TOKEN tipo_token = ProximoToken();
  TIPO_DATO tipo_semantico = T_DESCONOCIDO;

  // 1. Determinar el TIPO_DATO (Semántica)
  switch (tipo_token) {
    case TIPO_CHAR:
      tipo_semantico = T_CARACTER;
      break;
    case TIPO_FLOAT:
      tipo_semantico = T_REAL;
      break;
    case TIPO_INT:
      tipo_semantico = T_ENTERO;
      break;
    default:
      tipo_semantico = T_DESCONOCIDO;
      break;
  };

  // 2. Consumir el token de tipo (Sintaxis)
  Match(tipo_token);
  // 3. Procesar la lista de IDs, pasándole el tipo para que sean registradas en la TS
  ListaIdentificadores(tipo_semantico);
  // 4. Consumir el final
  Match(PUNTOYCOMA);
};

void ListaSentencias(void){
  /* <listaSentencias> -> <sentencia> {<sentencia>} */
  Sentencia();
  while(1){
    switch(ProximoToken()){
      case ID:
      case LEER:
      case ESCRIBIR:
      case MIENTRAS:
      case SI:
      case REPETIR:
        Sentencia();
        break;
      default: 
        return;
    };
  };
};

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
      ListaIdentificadores(T_DESCONOCIDO); // Se envía este parámetro para que se comporte como LECTURA.
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
    case MIENTRAS:
      SentenciaMientras();
      break;
    case SI:
      SentenciaSi();
      break;
    case REPETIR:
      SentenciaRepetirHasta();
    default:
      return;
  };
};

void ListaIdentificadores(TIPO_DATO tipo_asociado){
  /* <listaIdentificadores> -> <identificador> #leer_id {COMA <identificador> #leer_id} */
  TOKEN t;
  REG_EXPRESION reg;
  Identificador(&reg);

  if (tipo_asociado != T_DESCONOCIDO) { // Si es distinto, es porque estamos declarando.
    reg.tipo = tipo_asociado;
    Chequear(reg.nombre, reg.tipo); // Se registra el ID con el tipo de dato.
  } else {
    // Si no se está declarando, se está leyendo.
    Leer(reg);
  };

  for(t = ProximoToken(); t == COMA; t = ProximoToken()){
    Match(COMA);
    Identificador(&reg);
    if (tipo_asociado != T_DESCONOCIDO) { // Si es distinto, es porque estamos declarando.
      reg.tipo = tipo_asociado;
      Chequear(reg.nombre, reg.tipo); // Se registra el ID con el tipo de dato.
    } else {
    // Si no se está declarando, se está leyendo.
    Leer(reg);
    };
  };
};

void Identificador(REG_EXPRESION* presul){
  /* <identificador> -> ID #procesar_id */
  Match(ID);
  *presul = ProcesarId();
};

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
  };
};

void Expresion(REG_EXPRESION* presul){
  /* <expresion> -> <primaria> { <operadorAditivo> <primaria> #gen_infijo } */
  REG_EXPRESION operandoIzq, operandoDer;
  char op[TAMLEX];
  TOKEN t;
  Primaria(&operandoIzq);
  for(t = ProximoToken(); t == SUMA || t == RESTA; t = ProximoToken()){
    OperadorAditivo(op);
    Primaria(&operandoDer);
    operandoIzq = GenInfijo(operandoIzq, op, operandoDer);
  };
  *presul = operandoIzq;
};

void Primaria(REG_EXPRESION* presul){
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
    case CONSTANTE_CHAR:
      Match(CONSTANTE_CHAR);
      *presul = ProcesarCte(CONSTANTE_CHAR);
      break;
    case PARENIZQUIERDO:
      /* <primaria> -> PARENIZQUIERDO <expresion> PARENDERECHO */
      Match(PARENIZQUIERDO);
      Expresion(presul);
      Match(PARENDERECHO);
      break;
    default:
      return;
  };
};

void OperadorAditivo(char* presul){
  /* <operadorAditivo> -> SUMA #procesar_op | RESTA #procesar_op */
  TOKEN t = ProximoToken();
  if (t == SUMA || t == RESTA) {
    Match(t);
    strcpy(presul, ProcesarOp());
  } else ErrorSintactico(t);
};

void SentenciaSi(void){
  /* <sentencia_si> -> SI ( <condición> ) ENTONCES <listaSentencias> [SINO <listaSentencias>] FIN_SI ; */
  REG_EXPRESION cond;
  char* etiqueta_sino = NuevaEtiqueta();
  char* etiqueta_fin  = NuevaEtiqueta();

  Match(SI);
  Match(PARENIZQUIERDO);
  Condicion(&cond);
  Match(PARENDERECHO);
  // Consumir 'ENTONCES' y generar salto condicional
  Match(ENTONCES);
  Generar("Bf", cond.nombre, "", etiqueta_sino);
  // Cuerpo del SI (verdadero)
  ListaSentencias();
  // Procesar SINO (opcional)
  if(ProximoToken() == SINO){
    Generar("Br", "", "", etiqueta_fin);    // Salto al final del SI
    GenerarEtiqueta(etiqueta_sino);         // Etiqueta del SINO
    Match(SINO);
    ListaSentencias();
    GenerarEtiqueta(etiqueta_fin);          // Fin del SINO
  } else {
    GenerarEtiqueta(etiqueta_sino);         // Etiqueta del final si no hay SINO
  };

  Match(FIN_SI);
  free(etiqueta_sino);
  free(etiqueta_fin);
};

void SentenciaMientras(void){
  char *etiquetaInicio, *etiquetaFin;
  REG_EXPRESION condicion;
  TOKEN tok;

  Match(MIENTRAS);
  etiquetaInicio = NuevaEtiqueta();
  GenerarEtiqueta(etiquetaInicio); // Marca el comienzo del bucle

  Match(PARENIZQUIERDO);
  Condicion(&condicion);
  Match(PARENDERECHO);

  etiquetaFin = NuevaEtiqueta();
  Generar("Bf", Extraer(&condicion), etiquetaFin, ""); // Si la condición es falsa, salta al fin

  Match(HACER);

  tok = ProximoToken();
  while (tok != FIN_MIENTRAS && tok != FDT) {
    Sentencia();       // Consume el tokenActual
    tok = ProximoToken(); // Actualiza tok para la siguiente iteración
  };

  Generar("Bi", etiquetaInicio, "", ""); // Vuelve al inicio
  GenerarEtiqueta(etiquetaFin); // Fin del bucle

  Match(FIN_MIENTRAS);
  free(etiquetaInicio);
  free(etiquetaFin);
};

void SentenciaRepetirHasta(void){
  char *etiquetaInicio;
  REG_EXPRESION condicion;
  TOKEN tok;

  Match(REPETIR);

  etiquetaInicio = NuevaEtiqueta();
  GenerarEtiqueta(etiquetaInicio); // Marca el inicio del bucle

  // Ejecuta el cuerpo del repetir al menos una vez
  do {
    Sentencia();
    tok = ProximoToken();

    if (tok == PUNTOYCOMA) {
      Match(PUNTOYCOMA);
      tok = ProximoToken();
    }
  } while (tok != HASTA && tok != FDT);

  Match(HASTA);
  Match(PARENIZQUIERDO);
  Condicion(&condicion);
  Match(PARENDERECHO);

  // Si la condición es falsa, repetir el bucle
  Generar("Bf", Extraer(&condicion), etiquetaInicio, "");
  free(etiquetaInicio);
};

void Condicion(REG_EXPRESION* presul){
  REG_EXPRESION izq, der;
  char op[TAMLEX];
  Expresion(&izq);
  // Copiamos el operador relacional del buffer ANTES de hacer Match
  strcpy(op, ProcesarOp());
  Match(OP_RELACIONAL);
  //printf("%s\n", op);
  Expresion(&der);
  *presul = GenLogico(izq, op, der);
};

/********************** Rutinas semánticas ******************************/
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
  } else if (clase == CONSTANTE_CHAR) {
    reg.tipo = T_CARACTER;
    reg.valor_entero = buffer[1];
  };
  return reg;
};

REG_EXPRESION ProcesarId(void){
  /* Declara ID y construye el correspondiente registro semantico */
  REG_EXPRESION reg;
  TOKEN t;
  TIPO_DATO tipo_encontrado = T_DESCONOCIDO;

  // Buscar el identificador en la TS
  if (Buscar(buffer, TS, &t)) {
    // Si existe, recuperar su tipo de la TS
    int i = 0;
    while (strcmp("$", TS[i].identifi)) {
      if (!strcmp(buffer, TS[i].identifi)) {
        tipo_encontrado = TS[i].tipo;
        break;
      };
      i++;
    };
  };

  reg.clase = ID;
  strcpy(reg.nombre, buffer);
  reg.tipo = tipo_encontrado; // Asociar tipo semántico correcto
  return reg;
};

char* ProcesarOp(void){
  /* Declara OP y construye el correspondiente registro semantico */
  return buffer;
};

void Leer(REG_EXPRESION in){
  /* Genera la instruccion para leer */
  Generar("Read", in.nombre, TipoDatoToString(in.tipo), "");
};

void Escribir(REG_EXPRESION out){
  /* Genera la instruccion para escribir */
  Generar("Write", Extraer(&out), TipoDatoToString(out.tipo), "");
};

REG_EXPRESION GenInfijo(REG_EXPRESION e1, char* op, REG_EXPRESION e2){
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
};

REG_EXPRESION GenLogico(REG_EXPRESION e1, char* op, REG_EXPRESION e2){
  /* Genera código 3AC para comparación y retorna el temporal con el resultado */
  REG_EXPRESION reg;
  static unsigned int numTempLogico = 1;
  char cadTemp[TAMLEX] = "TempLog&";
  char cadNum[TAMLEX];
  char cadOp[TAMLEX];

  // Determinar la instrucción 3AC según el operador relacional
  if (!strcmp(op, "==")) strcpy(cadOp, "ComparaIGUAL");
  else if (!strcmp(op, "!=")) strcpy(cadOp, "ComparaDISTINTO");
  else if (!strcmp(op, ">")) strcpy(cadOp, "ComparaMAYOR");
  else if (!strcmp(op, "<")) strcpy(cadOp, "ComparaMENOR");
  else if (!strcmp(op, ">=")) strcpy(cadOp, "ComparaMAYORIGUAL");
  else if (!strcmp(op, "<=")) strcpy(cadOp, "ComparaMENORIGUAL");
  else {
    printf("ERROR SEMANTICO: Operador relacional desconocido en GenLogico: '%s'\n", op);
    strcpy(cadOp, "ComparaDESCONOCIDO");
  }

  // Crear un nuevo temporal
  sprintf(cadNum, "%d", numTempLogico++);
  strcat(cadTemp, cadNum);

  // Declarar el temporal y generar la instrucción
  Chequear(cadTemp, T_ENTERO); // Resultado lógico es siempre entero (0 o 1)
  if(e1.clase == ID) Chequear(Extraer(&e1), e1.tipo);
  if(e2.clase == ID) Chequear(Extraer(&e2), e2.tipo);

  Generar(cadOp, Extraer(&e1), Extraer(&e2), cadTemp);

  strcpy(reg.nombre, cadTemp);
  reg.tipo = T_ENTERO;
  return reg;
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
  if (izq.tipo == T_DESCONOCIDO || der.tipo == T_DESCONOCIDO) 
    ErrorSemantico("Uso de identificador no declarado o tipo desconocido.");
  if (izq.tipo != der.tipo) {
    char mensaje[128];
    sprintf(mensaje, "Asignacion incompatible (%s := %s)", 
      TipoDatoToString(izq.tipo), 
      TipoDatoToString(der.tipo));
    ErrorSemantico(mensaje);
  };
  Chequear(izq.nombre, der.tipo);
  Generar("Almacena", Extraer(&der), izq.nombre, "");
};

/*************** Funciones auxiliares **********************************/
void Match(TOKEN t){
  if(!(t == ProximoToken())) ErrorSintactico();
  flagToken = 0;
};

TOKEN ProximoToken(){
  if (!flagToken) {
    tokenActual = scanner();
    if(tokenActual == ERRORLEXICO) ErrorLexico();
    flagToken = 1;
    if (tokenActual == ID) {
      Buscar(buffer, TS, &tokenActual);
    };
  };
  return tokenActual;
      /*
      if (!flagToken) {
        tokenActual = scanner();
        if(tokenActual == ERRORLEXICO) {
          ErrorLexico();
          // opcional: mostrar buffer para ver qué produjo el error
          fprintf(stderr, "DEBUG scanner -> ERRORLEXICO, buffer='%s'\n", buffer);
        }
        flagToken = 1;
        
        // Si es ID, la búsqueda puede convertirlo en palabra reservada
        if (tokenActual == ID) {
          // Guardamos el token devuelto por la TS en tokenActual
          Buscar(buffer, TS, &tokenActual);
        }
        
        // DEBUG: imprimir qué token fue producido y cuál es su lexema
        // Usamos stderr para no mezclar con la salida normal del compilador (stdout)
        fprintf(stderr, "DEBUG ProximoToken -> token=%d lexema='%s'\n", tokenActual, buffer);
      }
      return tokenActual;
      */
};

void ErrorLexico(){
  printf("Error Lexico\n");
};

void ErrorSintactico(){
  printf("Error Sintactico\n");
};

void ErrorSemantico(const char* mensaje) {
  fprintf(stderr, "Error Semantico: %s\n", mensaje);
  exit(1); // Termina el compilador
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
  if (!Buscar(s, TS, &t)) {
    Colocar(s, TS, tipo);
    Generar("Declara", s, TipoDatoToString(tipo), "");
  };
};

char* TipoDatoToString(TIPO_DATO tipo){
  if (tipo == T_ENTERO) return "Entera";
  if (tipo == T_CARACTER) return "Caracter";
  if (tipo == T_REAL) return "Real";
  return "Desconocida";
};

char* NuevaEtiqueta(void){
  static unsigned int numEtiqueta = 1;
  char cadTemp[TAMLEX];
  sprintf(cadTemp, "L%d", numEtiqueta++);
  return strdup(cadTemp);
};

void GenerarEtiqueta(char* e){
  printf("%s:\n", e);
};