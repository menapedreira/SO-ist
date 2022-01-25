/*--------------------------------------------------------------------
| Projeto SO - exercicio 3
| Grupo 87
| Madalena Pedreira   86466 
| Rita Fernandes  86508
---------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <pthread.h>

#include "matrix2d.h"


typedef struct {
  int id;
  int n_trab;
  int n_colunas;
  int n_linhas;
  int iteracoes;
} argsThread;


/*--------------------------------------------------------------------
| Global Variables
---------------------------------------------------------------------*/

DoubleMatrix2D  *Mestre, *Mestre_aux;
int counter; /*quando estiver a iteracoes*trab pode atualizar a mestre com novos valores calculados*/
int flag; /*flag que quando estiver a 0 faz uma e uma so vez por iteracao a troca de ponteiros entre a matriz Mestre e a Mestre_aux*/
double maxD, maxDif;
pthread_mutex_t mutex;
pthread_cond_t cond;



/*--------------------------------------------------------------------
| Function: simul
---------------------------------------------------------------------*/


void *simul(void *a) {

  DoubleMatrix2D  *tmp;
  argsThread*     thread;  
  int             i, j, iter, n_iter ,linhas,colunas, id,trab, begin, end, limite;
  double          value, dif,max_dif;


  thread = (argsThread*) a;
  linhas = thread->n_linhas;
  colunas = thread->n_colunas;
  id = thread->id;
  n_iter = thread->iteracoes;
  trab = thread->n_trab;
  begin = (linhas)*(id-1)+1; /*inico da fatia*/
  end = (linhas)*(id-1) + linhas ; /*fim da fatia*/
  



  for (iter=0; iter< n_iter; iter++){
   
    max_dif = 0.0;
    limite = (iter+1)*trab;
    for (i = begin; i < end+ 1; i++){
      for (j = 1; j < colunas - 1; j++) {
        value = ( dm2dGetEntry(Mestre, i-1, j) + dm2dGetEntry(Mestre, i+1, j) +
      dm2dGetEntry(Mestre, i, j-1) + dm2dGetEntry(Mestre, i, j+1) ) / 4.0;
        dm2dSetEntry(Mestre_aux, i, j, value);

        dif= dm2dGetEntry(Mestre_aux,i,j)-dm2dGetEntry(Mestre,i,j);
        if(dif>max_dif)
          max_dif=dif;

      }
    }
   
    if (max_dif>maxDif)
      maxDif = max_dif;

    pthread_mutex_lock(&mutex); 
    counter++;
     flag = 0;

    while(counter < limite){
        pthread_cond_wait(&cond,&mutex);
    }
    
    

    if (counter == limite){         /*espera pelo ultimo thread e se a flag nao acusar maxD faz as trocas normais*/
      if(flag==0){
      flag = 1;
      tmp = Mestre_aux;
      Mestre_aux = Mestre;
      Mestre = tmp;
      pthread_cond_broadcast(&cond);
      }
      if(maxDif<maxD){  /*condicao de paragem maxDif < maxD*/
      pthread_cond_broadcast(&cond);
      pthread_mutex_unlock(&mutex);
      return NULL;
     }
      maxDif = 0;
    }

    
    
     
   pthread_mutex_unlock(&mutex);

  }

  pthread_exit(NULL);
 
}

/*---------------------------------------------------------------------------
createThread
-----------------------------------------------------------------------------*/
int createThread(int trab, int iteracoes) {

  int             i, linhas_por_fatia;
  pthread_t       *mainThread; 
  argsThread      *thread_args;
  

  linhas_por_fatia = (Mestre->n_c -2)/trab; /*Calcula o numero de linhas por fatia horizontal interior*/
 


  if ((mainThread = (pthread_t*)malloc(trab*sizeof(pthread_t))) == NULL ) {
    printf("\nErro na alocacao de memoria de mainThread.\n\n");
    return -1;
  }

  if ((thread_args = (argsThread*)malloc(trab*sizeof(argsThread))) == NULL ) {
    printf("\nErro na alocacao de memoria de thread_args.\n\n");
    return -1;
  }


  for (i=0; i<trab; i++) {
    thread_args[i].id=i+1;
    thread_args[i].n_trab=trab;
    thread_args[i].n_colunas=Mestre->n_c;
    thread_args[i].n_linhas=linhas_por_fatia;
    thread_args[i].iteracoes=iteracoes;

    pthread_create(&mainThread[i], NULL, simul, &thread_args[i]);
  }


  for (i=0; i<trab; i++) {
    if (pthread_join(mainThread[i], NULL)) {
      fprintf(stderr, "\nErro ao esperar por um escravo.\n");    
      return -1;
    }  
  }

  free(thread_args);
  free(mainThread);
  return(1);

}
/*--------------------------------------------------------------------
| Function: parse_integer_or_exit
---------------------------------------------------------------------*/

int parse_integer_or_exit(char const *str, char const *name)
{
  int value;
 
  if(sscanf(str, "%d", &value) != 1) {
    fprintf(stderr, "\nErro no argumento \"%s\".\n\n", name);
    exit(1);
  }
  return value;
}

/*--------------------------------------------------------------------
| Function: parse_double_or_exit
---------------------------------------------------------------------*/

double parse_double_or_exit(char const *str, char const *name)
{
  double value;

  if(sscanf(str, "%lf", &value) != 1) {
    fprintf(stderr, "\nErro no argumento \"%s\".\n\n", name);
    exit(1);
  }
  return value;
}


/*--------------------------------------------------------------------
| Function: main
---------------------------------------------------------------------*/

int main (int argc, char** argv) {

  if(argc != 9) {
    fprintf(stderr, "\nNumero invalido de argumentos.\n");
    fprintf(stderr, "Uso: heatSim N tEsq tSup tDir tInf iteracoes trab\n\n");
    return 1;
  }
  
  /* argv[0] = program name */
  int N = parse_integer_or_exit(argv[1], "N");
  double tEsq = parse_double_or_exit(argv[2], "tEsq");
  double tSup = parse_double_or_exit(argv[3], "tSup");
  double tDir = parse_double_or_exit(argv[4], "tDir");
  double tInf = parse_double_or_exit(argv[5], "tInf");
  int iteracoes = parse_integer_or_exit(argv[6], "iteracoes");
  int trab = parse_integer_or_exit(argv[7],"trab");
  maxD=parse_double_or_exit(argv[8],"maxD");

  counter = 0;
  maxDif = 0;
  flag = 0;
 

      

  fprintf(stderr, "\nArgumentos:\n"
  " N=%d tEsq=%.1f tSup=%.1f tDir=%.1f tInf=%.1f iteracoes=%d tarefas=%d maxD=%.1f \n",
  N, tEsq, tSup, tDir, tInf, iteracoes,trab,maxD);

  if(N < 1 || tEsq < 0 || tSup < 0 || tDir < 0 || tInf < 0 || iteracoes < 1 || trab<1 ||(N%trab)!=0) {
    fprintf(stderr, "\nErro: Argumentos invalidos.\n"
  " Lembrar que N > 0 e tem que ser divisivel por tarefas, temperaturas >= 0, iteracoes > 0, tarefas > 0.\n\n");
    return 1;
  }

 
  



  if ((Mestre = dm2dNew(N+2,N+2)) == NULL) {
    printf("\nErro na alocacao de memoria da matriz Mestre.\n\n");
    return -1;
  }
  

  if ((Mestre_aux = dm2dNew(N+2,N+2)) == NULL) {
    printf("\nErro na alocacao de memoria da matriz Mestre_aux.\n\n");
    return -1;
  }
  

  /*Atribuicao de valores a matriz Mestre*/
  int     i;
  for(i=0;i<N+2;i++){
    dm2dSetLineTo(Mestre, i, 0);
  }
  
  dm2dSetLineTo (Mestre, 0, tSup);
  dm2dSetLineTo (Mestre,N+1, tInf);

  dm2dSetColumnTo (Mestre, 0, tEsq);
  dm2dSetColumnTo (Mestre, N+1, tDir);
  

  dm2dCopy(Mestre_aux,Mestre);

  createThread(trab,iteracoes);

  dm2dPrint(Mestre);

  dm2dFree(Mestre);
  dm2dFree(Mestre_aux);

  return 0;
}
