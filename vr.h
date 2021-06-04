#include <stdio.h>
#include <string.h>

#include <R.h>
#include <Rembedded.h>
#include <Rinterface.h>
#include <Rinternals.h>
#include <Rdefines.h>
#include <Rversion.h>


/* From Parse.h -- must find better solution: */
#define PARSE_NULL              0
#define PARSE_OK                1
#define PARSE_INCOMPLETE        2
#define PARSE_ERROR             3
#define PARSE_EOF               4


#define Need_Integer(x) (x) = rb_Integer(x)
#define Need_Float(x) (x) = rb_Float(x)
#define Need_Float2(x,y) {\
    Need_Float(x);\
    Need_Float(y);\
}
#define Need_Float3(x,y,z) {\
    Need_Float(x);\
    Need_Float(y);\
    Need_Float(z);\
}

#if (R_VERSION < 132352) /* before 2.5 to check!*/
SEXP R_ParseVector(SEXP, int, int *);
#define RR_ParseVector(x,y,z) R_ParseVector(x, y, z)
#else
SEXP R_ParseVector(SEXP, int, int *,SEXP);
#define RR_ParseVector(x,y,z) R_ParseVector(x, y, z, R_NilValue)
#endif

/************* INIT *********************/

extern Rboolean R_Interactive;
extern uintptr_t R_CStackLimit; /* C stack limit */
extern uintptr_t R_CStackStart; /* Initial stack address */
//extern int Rf_initEmbeddedR(int argc, char *argv[]);
static int vr_initialized=0;

int vr_init() //(int argc,char* argv[])
{
  if(!vr_initialized) {
    char* argv[4];
    argv[0]="REmbed";
    argv[1]="--save";
    argv[2]="--slave";
    argv[3]="--quiet";
    R_CStackStart = (uintptr_t)-1;
    Rf_initEmbeddedR(4,argv);
    R_Interactive = FALSE;
    printf("R init\n");
    vr_initialized=1;
    return 1;
  } else return 0;
}

/***************** EVAL **********************/

int vr_eval(char* cmds, int print)
{
  int nbCmds,errorOccurred,status, i;

  SEXP text, expr, ans=R_NilValue /* -Wall */;


  //printf("Avant parsing\n");

  nbCmds=1;

  //printf("nbCmds : %d\n",nbCmds);
  //printf("%s\n",cmds);

  text = PROTECT(allocVector(STRSXP, nbCmds));
  for (i = 0 ; i < nbCmds ; i++) {
    SET_STRING_ELT(text, i, mkChar(cmds));
  }
  expr = PROTECT(RR_ParseVector(text, -1, &status));

  if (status != PARSE_OK) {
    //printf("Parsing error (status=%d) in:\n",status);
    for (i = 0 ; i < nbCmds ; i++) {
      //printf("%s\n",cmds);
    }
    UNPROTECT(2);
    return 0;
  }
  
  /* Note that expr becomes an EXPRSXP and hence we need the loop
     below (a straight eval(expr, R_GlobalEnv) won't work) */
  {
    for(i = 0 ; i < nbCmds ; i++)
      ans = R_tryEval(VECTOR_ELT(expr, i),NULL, &errorOccurred);
      if(errorOccurred) {
        //fprintf(stderr, "Caught another error calling sqrt()\n");
        fflush(stderr);
        UNPROTECT(2);
        return 0;
      }

      if (print) {
        Rf_PrintValue(ans);
      }
  }

  UNPROTECT(2);
  return 1;
}

//array 
void* util_SEXP2C(SEXP ans,int* type,int* len) {
  *len=length(ans);
  void* res;
  switch(TYPEOF(ans)) {
  case REALSXP:
    *type=0;
    //printf("type is real\n");
    res=(void*)(REAL(ans));
    break;
  case INTSXP:
    *type=1;
    res=(void*)(INTEGER(ans));
    break;
  case LGLSXP:
    *type=2;
    res=(void*)(INTEGER(ans));
    break;
  case STRSXP:
    *type=3;
    char** resC=malloc((*len) * sizeof(char*));
    for(int i=0;i<(*len);i++) {
      resC[i]=(char *)CHAR(STRING_ELT(ans,i));
    }
    res=(void*)resC;
    break;
  // case CPLXSXP:
  //   rb_require("complex");
  //   for(i=0;i<n;i++) {
  //     Rcomplex cpl=COMPLEX(ans)[i];
  //     res2 = rb_eval_string("Complex.new(0,0)");
  //     rb_iv_set(res2,"@real",rb_float_new(cpl.r));
  //     rb_iv_set(res2,"@image",rb_float_new(cpl.i));
  //     rb_ary_store(res,i,res2);
  //   }
  //   break;
  }

  return res;
}

// 
void* vr_get_ary(char* cmd,int* type,int* len) {
  int  errorOccurred,status, i;
    
  SEXP text, expr, ans; //=R_NilValue /* -Wall */;

  text = PROTECT(allocVector(STRSXP, 1)); 
//printf("cmd: %s\n",cmdString);
  SET_STRING_ELT(text, 0, mkChar(cmd));
  expr = PROTECT(RR_ParseVector(text, -1, &status));
  if (status != PARSE_OK) {
    printf("Parsing error in: %s\n",cmd);
    UNPROTECT(2);
    return (void*)NULL;
  }
  /* Note that expr becomes an EXPRSXP and hence we need the loop
     below (a straight eval(expr, R_GlobalEnv) won't work) */
  ans = R_tryEval(VECTOR_ELT(expr, 0),R_GlobalEnv,&errorOccurred);
  if(errorOccurred) {
    //fflush(stderr);
    printf("Exec error in: %s\n",cmd);
    UNPROTECT(2);
    return (void*)NULL;
  }
  UNPROTECT(2);
  //printf("eval_get\n");
  return util_SEXP2C(ans,type,len);
}

double* vr_as_double_ary(void* res) {
   return (double*)res;
}

int* vr_as_int_ary(void* res) {
   return (int*)res;
}

char** vr_as_string_ary(void* res) {
   return (char**)res;
}

SEXP util_C2SEXP(void* arr,int type,int n) {
  SEXP ans;
  int i; 
  
  if(type==0) {
    PROTECT(ans=allocVector(REALSXP,n));
    for(i=0;i<n;i++) {
      REAL(ans)[i]=((double*)arr)[i];
    }
    UNPROTECT(1);
  } else if(type==1) {
    PROTECT(ans=allocVector(INTSXP,n));
    for(i=0;i<n;i++) {
      INTEGER(ans)[i]=((int*)arr)[i];
    }
    UNPROTECT(1);
  } else if(type==2) {
    PROTECT(ans=allocVector(LGLSXP,n));
    for(i=0;i<n;i++) {
      LOGICAL(ans)[i]=((int*)arr)[i];
    }
    UNPROTECT(1);
  } else if(type==3) {
    PROTECT(ans=allocVector(STRSXP,n));
    for(i=0;i<n;i++) {
      char* str=((char**)arr)[i];
      //printf("[%d]=%s|\n",i,str);
      SET_STRING_ELT(ans,i,mkCharCE(str,CE_UTF8));
    }
    UNPROTECT(1);
  } else ans=R_NilValue;

  return ans; 
}

void vr_set_ary(char* name, void* arr, int type, int len) {
  SEXP ans=util_C2SEXP(arr,type,len);
  defineVar(install(name),ans,R_GlobalEnv);
}