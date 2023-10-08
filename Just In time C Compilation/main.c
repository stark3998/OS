/**
 * Tony Givargis
 * Copyright (C), 2023
 * University of California, Irvine
 *
 * CS 238P - Operating Systems
 * main.c
 */

#include "jitc.h"
#include "parser.h"
#include "system.h"
#include "math.h"

/* export LD_LIBRARY_PATH=. */

static void reflect(const struct parser_dag *dag, FILE *file)
{
  if (dag)
  {
    reflect(dag->left, file);
    reflect(dag->right, file);
    if (PARSER_DAG_VAL == dag->op)
    {
      fprintf(file, "double t%d = %f;\n", dag->id, dag->val);
    }
    else if (PARSER_DAG_NEG == dag->op)
    {
      fprintf(file, "double t%d = - t%d;\n", dag->id, dag->right->id);
    }
    else if (PARSER_DAG_MUL == dag->op)
    {
      fprintf(file, "double t%d = t%d * t%d;\n", dag->id, dag->left->id,
              dag->right->id);
    }
    else if (PARSER_DAG_DIV == dag->op)
    {
      fprintf(file, "double t%d = t%d ? (t%d / t%d) : 0.0;\n", dag->id,
              dag->right->id, dag->left->id, dag->right->id);
    }
    else if (PARSER_DAG_ADD == dag->op)
    {
      fprintf(file, "double t%d = t%d + t%d;\n", dag->id, dag->left->id,
              dag->right->id);
    }
    else if (PARSER_DAG_SUB == dag->op)
    {
      fprintf(file, "double t%d = t%d - t%d;\n", dag->id, dag->left->id,
              dag->right->id);
    }
    else
    {
      EXIT("Invalid Expression");
    }
  }
}

double sigmoid(double x)
{
  return 1 / (1 + exp(-x));
}

static void generate_c(const struct parser_dag *dag, FILE *file)
{
  fprintf(file, "typedef double (*sigmoid_t)(double);\n\n");
  fprintf(file, "double evaluate(sigmoid_t sigmoid_fn) {\n");
  reflect(dag, file);
  fprintf(file, "return sigmoid_fn(t%d);\n}\n", dag->id);
}

typedef double (*evaluate_t)(double(double));

int main(int argc, char *argv[])
{
  const char *SOFILE = "out.so";
  const char *CFILE = "out.c";
  struct parser *parser;
  struct jitc *jitc;
  evaluate_t fnc;
  FILE *file;

  /* usage */

  if (2 != argc)
  {
    printf("One Expression Argument Expected: %s expression\n", argv[0]);
    return -1;
  }

  /* parse */

  if (!(parser = parser_open(argv[1])))
  {
    TRACE("Unable to generate a parser object");
    return -1;
  }

  /* generate C */

  if (!(file = fopen(CFILE, "w")))
  {
    TRACE("Unable to Open C File");
    return -1;
  }

  generate_c(parser_dag(parser), file);
  parser_close(parser);
  fclose(file);

  /* JIT compile */
  if (jitc_compile(CFILE, SOFILE))
  {
    file_delete(CFILE);
    TRACE(0);
    return -1;
  }

  file_delete(CFILE);

  /* dynamic load */

  if (!(jitc = jitc_open(SOFILE)) ||
      !(fnc = (evaluate_t)jitc_lookup(jitc, "evaluate")))
  {
    file_delete(SOFILE);
    jitc_close(jitc);
    TRACE(0);
    return -1;
  }

  printf("%f\n", fnc(&sigmoid));

  /* done */

  file_delete(SOFILE);
  jitc_close(jitc);
  return 0;
}
