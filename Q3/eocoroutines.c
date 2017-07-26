#include <stdio.h>
#include <setjmp.h>

jmp_buf evens_env, odds_env;

int array[10];
int arrpos = 0; //position in array
int epos, opos; //helper
int numprinted;
int odds_not_called = 1; //if first time in evens, let us know that odds has not been called

void odds(); //forward declaration

void evens(){
  while(numprinted < 10){
    if(array[arrpos] % 2 == 0){
      printf("%i ", array[arrpos]);
      arrpos++;
      numprinted++;
    }
    else{
        epos = setjmp(evens_env);
        if(epos == 0) {
          if(odds_not_called){ odds_not_called = 0; odds(); } //we call odds for the first time only
          else { longjmp(odds_env,1); }
        }
    }
  }
}

void odds(){
  while(numprinted < 10){
    if(array[arrpos] % 2 != 0){
      printf("%i ", array[arrpos]);
      arrpos++;
      numprinted++;
    }
    else{
      opos = setjmp(odds_env);
      if(opos == 0) { longjmp(evens_env,1); }
    }
  }
}

int main(int argc, char** argv){
  printf("Enter 10 integers: \n");
  scanf("%d %d %d %d %d %d %d %d %d %d",
    &array[0], &array[1], &array[2], &array[3], &array[4], &array[5], &array[6], &array[7], &array[8], &array[9]);
  evens();
}
