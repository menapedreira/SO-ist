/*--------------------------------------------------------------------
| Projeto SO - exercicio 1
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
#include "mplib3.h"
#include "leQueue.h"

typedef struct {
  int id;
  int n_trab;
  int n_colunas;
  int n_linhas;
  int iteracoes;
} argsThread;


/*--------------------------------------------------------------------
| Function: mpEntreTarefas
---------------------------------------------------------------------*/

void mpEntreTarefas(int id, void *a, int linhas, int mess_size, int trab){
  DoubleMatrix2D  *aux;

  aux = (DoubleMatrix2D*) a;

  if(id==1){
    receberMensagem(id+1,id,dm2dGetLine(aux,linhas+1),mess_size);
    enviarMensagem(id,id+1,dm2dGetLine(aux,linhas),mess_size);
  }

  else if(id<trab && id>1){
    if(id%2==0){
      enviarMensagem(id,id-1, dm2dGetLine(aux,1),mess_size);
      receberMensagem(id-1,id,dm2dGetLine(aux,0),mess_size);
      enviarMensagem(id,id+1,dm2dGetLine(aux,linhas),mess_size);
      receberMensagem(id+1,id,dm2dGetLine(aux,linhas+1),mess_size);
    }
    else{
      receberMensagem(id+1,id,dm2dGetLine(aux,linhas+1),mess_size);          
      enviarMensagem(id,id+1,dm2dGetLine(aux,linhas),mess_size);
      receberMensagem(id-1,id,dm2dGetLine(aux,0),mess_size);
      enviarMensagem(id,id-1, dm2dGetLine(aux,1),mess_size);
    }
  }
  
  else{
    if(id%2==0){
      enviarMensagem(id,id-1, dm2dGetLine(aux,1),mess_size);
      receberMensagem(id-1,id,dm2dGetLine(aux,0),mess_size);
    }
    else{
      receberMensagem(id-1,id,dm2dGetLine(aux,0),mess_size);
      enviarMensagem(id,id-1, dm2dGetLine(aux,1),mess_size);
    }
  } 
}


/*--------------------------------------------------------------------
| Function: simul
---------------------------------------------------------------------*/


void *simul(void *a) {

  DoubleMatrix2D  *m,*aux, *tmp;
  argsThread*     thread;  
  int             i, j, iter, n_iter ,linhas,colunas,mess_size, id,trab;
  double          value;


  thread = (argsThread*) a;
  linhas = thread->n_linhas;
  colunas = thread->n_colunas;
  mess_size = (colunas)*(sizeof(double));
  id = thread->id;
  n_iter = thread->iteracoes;
  trab = thread->n_trab;


  if((m=dm2dNew(linhas+2,colunas)) == NULL){
    printf("\nErro na alocacao de memoria da matriz m.\n\n");
    return NULL;
  }
  if((aux=dm2dNew(linhas+2,colunas)) == NULL){
    printf("\nErro na alocacao de memoria da matriz aux.\n\n");
    return NULL;
  }
  
  
  receberMensagem(0,id,dm2dGetLine(m,0),mess_size*(linhas+2));

  dm2dCopy(aux,m);
  
  for (iter=0; iter<n_iter; iter++){

    for (i = 1; i < linhas + 1; i++){
      for (j = 1; j < colunas - 1; j++) {
        value = ( dm2dGetEntry(m, i-1, j) + dm2dGetEntry(m, i+1, j) +
      dm2dGetEntry(m, i, j-1) + dm2dGetEntry(m, i, j+1) ) / 4.0;
        dm2dSetEntry(aux, i, j, value);
      }
    }
    if (trab != 1){
      mpEntreTarefas(id, aux, linhas, mess_size, trab);
    }  

    tmp = aux;
    aux = m;
    m = tmp;
   
  }

  enviarMensagem(id,0,dm2dGetLine(m,1),mess_size*linhas);


  dm2dFree(m);
  dm2dFree(aux);
  pthread_exit(NULL);
 
}


/*---------------------------------------------------------------------------
createThread
-----------------------------------------------------------------------------*/
DoubleMatrix2D* createThread(int trab, int iteracoes,DoubleMatrix2D *Mestre) {

  int             i, j, linhas_por_fatia, id, mess_size;
  pthread_t       *mainThread; 
  argsThread      *thread_args;
  

  linhas_por_fatia = (Mestre->n_c -2)/trab; //Calcula o numero de linhas por fatia horizontal interior
  mess_size=(Mestre->n_c)*(sizeof(double)); //Calcula o tamanho de cada linha


  if ((mainThread = (pthread_t*)malloc(trab*sizeof(pthread_t))) == NULL ) {
    printf("\nErro na alocacao de memoria de mainThread.\n\n");
    return NULL;
  }

  if ((thread_args = (argsThread*)malloc(trab*sizeof(argsThread))) == NULL ) {
    printf("\nErro na alocacao de memoria de thread_args.\n\n");
    return NULL;
  }


  for (i=0; i<trab; i++) {
    thread_args[i].id=i+1;
    thread_args[i].n_trab=trab;
    thread_args[i].n_colunas=Mestre->n_c;
    thread_args[i].n_linhas=linhas_por_fatia;
    thread_args[i].iteracoes=iteracoes;

    pthread_create(&mainThread[i], NULL, simul, &thread_args[i]);
  }


  //Troca de mensagens entre thread mestre (id = 0) e sub_threads

  for (i=1;i<=trab;i++)
  {
    enviarMensagem(0,i,dm2dGetLine(Mestre,linhas_por_fatia*(i-1)),mess_size*(linhas_por_fatia+2));
}

  for (i=1;i<=trab;i++){
    receberMensagem(i,0,dm2dGetLine(Mestre,((i-1)*linhas_por_fatia)+1),mess_size*linhas_por_fatia);
    }



  for (i=0; i<trab; i++) {
    if (pthread_join(mainThread[i], NULL)) {
      fprintf(stderr, "\nErro ao esperar por um escravo.\n");    
      return NULL;
    }  
  }

  free(thread_args);
  free(mainThread);
  return Mestre;

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
    fprintf(stderr, "Uso: heatSim N tEsq tSup tDir tInf iteracoes trab csz\n\n");
    return 1;
  }
  
  /* argv[0] = program name */
  int N = parse_integer_or_exit(argv[1], "N");
  double tEsq = parse_double_or_exit(argv[2], "tEsq");
  double tSup = parse_double_or_exit(argv[3], "tSup");
  double tDir = parse_double_or_exit(argv[4], "tDir");
  double tInf = parse_double_or_exit(argv[5], "tInf");
  int iteracoes = parse_integer_or_exit(argv[6], "iteracoes");
  int trab=parse_integer_or_exit(argv[7],"trab");
  int csz=parse_integer_or_exit(argv[8],"csz");
  


  fprintf(stderr, "\nArgumentos:\n"
  " N=%d tEsq=%.1f tSup=%.1f tDir=%.1f tInf=%.1f iteracoes=%d tarefas=%d msg_por_canal=%d\n",
  N, tEsq, tSup, tDir, tInf, iteracoes,trab,csz);

  if(N < 1 || tEsq < 0 || tSup < 0 || tDir < 0 || tInf < 0 || iteracoes < 1 || trab<1 ||csz<0||(N%trab)!=0) {
    fprintf(stderr, "\nErro: Argumentos invalidos.\n"
  " Lembrar que N > 0 e tem que ser divisivel por tarefas, temperaturas >= 0, iteracoes > 0, tarefas > 0, msg_por_canal >= 0.\n\n");
    return 1;
  }

  if(inicializarMPlib(csz,trab+1)==-1){
    fprintf(stderr,"\nErro ao inicializar MPLib.\n\n");
    return -1;
  }

  
  DoubleMatrix2D  *Mestre,*result;
  int             i;


  if ((Mestre = dm2dNew(N+2,N+2)) == NULL) {
    printf("\nErro na alocacao de memoria da matriz Mestre.\n\n");
    return -1;
  }

  if ((result = dm2dNew(N+2,N+2)) == NULL) {
    printf("\nErro na alocacao de memoria da matriz result.\n\n");
    return -1;
  }
  

  //Atribuicao de valores a matriz Mestre

  for(i=0;i<N+2;i++){
    dm2dSetLineTo(Mestre, i, 0);
  }
  
  dm2dSetLineTo (Mestre, 0, tSup);
  dm2dSetLineTo (Mestre,N+1, tInf);

  dm2dSetColumnTo (Mestre, 0, tEsq);
  dm2dSetColumnTo (Mestre, N+1, tDir);
  

  dm2dCopy(result,createThread(trab,iteracoes,Mestre));
 
  
  if (result == NULL) {
    printf("\nErro na simulacao.\n\n");
    return -1;
  }


  dm2dPrint(result);

  dm2dFree(result);
  dm2dFree(Mestre);
  libertarMPlib();

  return 0;
}
