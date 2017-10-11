#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <stdint.h>
#include <errno.h>
//#include <sys/types.h>
#include <math.h>
#include <stdbool.h>
#include <arpa/inet.h>

#define DEBUG 2

#define NON_ELEM 0

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_RESET   "\x1b[0m"

#if defined(DEBUG) && DEBUG > 1
#define trace(fmt, args...) fprintf(stderr, ANSI_COLOR_GREEN "DEBUG %s:%d:%s(): " ANSI_COLOR_RESET fmt "\n", __FILE__, __LINE__, __func__, ##args)
#else
#define trace(fmt, args...) /* Don't do anything in release builds */
#endif

#if defined(DEBUG) && DEBUG > 0
#define rtrace(fmt, args...) fprintf(stderr, ANSI_COLOR_YELLOW "DEBUG %s:%d:%s(): " ANSI_COLOR_RESET fmt "\n", __FILE__, __LINE__, __func__, ##args)
#else
#define rtrace(fmt, args...) /* Don't do anything in release builds */
#endif

#if defined(DEBUG) && DEBUG > 0
#define err(fmt, args...) fprintf(stderr, ANSI_COLOR_RED "DEBUG %s:%d:%s(): " ANSI_COLOR_RESET fmt "\n", __FILE__, __LINE__, __func__, ##args)
#else
#define err(fmt, args...) /* Don't do anything in release builds */
#endif

//run limits
#define M_SET_MAX 10
#define N_ELEM_MAX 20000000
#define INPUT_FILE_EXTENSION ".dat"

struct rec
{
  unsigned int x;
};

unsigned int recursive(int m, int n, int k, int * ibegin, int * iend, FILE * file[]);
int endElem(int * endArray, int * begArray, int length, bool big_small);
int int_cmp(const void *a, const void *b);
int binsearch(int ibegin, int iend, unsigned int radix, FILE * file[], int m);

int main(int argc, char * argv[]) // <prog> [input.txt] [output.txt]
{
  if (argc <= 2)
  {
    err("invalid arguements, argc :: %d ::", argc);
    exit(1);
  }
  //stuff for the algorithm
  int k = N_ELEM_MAX; //the # of the smallest we want
  int m = M_SET_MAX; // the number of sorted sets
  int n = N_ELEM_MAX; // the maximum lenght of a sorted set
  FILE * file[M_SET_MAX]; //fd for m-sets
  FILE * fd;
  char * line = NULL;
  size_t len = (m*k); //ohh no...
  ssize_t read_len;
  int i = 0;
  char input_filename[(int)((ceil(log10(M_SET_MAX))+1)*sizeof(char)+sizeof(INPUT_FILE_EXTENSION))];
  //
  //trace("opening argv[1] :: %s ::", argv[1]);
  fd = fopen(argv[1], "r");
  read_len = getline(&line, &len, fd);
  //trace("Retrieved line of length %zu", read_len);
  //trace("Line is :: %s", line);
  sscanf(line, "%d,%d,%d", &m, &n, &k);
  fclose(fd);
  //trace("m, n, k, are now :: m = %d :: n = %d :: k = %d ::", m, n, k); 
  //trace("cwd :: %s ::",  get_current_dir_name());
  for (i = 0; i < m; i++) 
  {
    sprintf(input_filename, "%d", (i+1));
    trace("Opening :: %s.dat ::", input_filename);
    file[i] = fopen(strcat(input_filename, ".dat"), "r");
    trace("strerror :: %s ::", strerror(errno));
  }
  //
  int * ibegin = (int * )calloc(m, sizeof(int)); //tracks the beg..
  int * iend = (int *)calloc(m, sizeof(int)); //tracks the end
  for (i = 0; i < m; i++) 
  {
    iend[i] = (n-1);
  }
  trace("testing ith of beg and end ::beg %d ::end %d",ibegin[0], iend[m-1]);
  //stuff for this driver
  struct timeval  tv1, tv2;
  gettimeofday(&tv1, NULL);
  /* Do sutff */
  trace("Entering Hell (%d) :: m = %d :: n = %d :: k = %d ::", getpid(), m, n, k);
  printf("The k-th smallest element is :: %u ::", recursive(m,n,k,ibegin,iend,file));
  /* nothing past this moment */
  gettimeofday(&tv2, NULL);
  trace("Total time = %f seconds", (double) (tv2.tv_usec - tv1.tv_usec) / 1000000 + (double) (tv2.tv_sec - tv1.tv_sec));

  //cleanup
  for (i = 0; i < m; i++) 
  {
    trace("Closing :: %d.dat ::", (i+1));
    fclose(file[i]);
  }
  //TIME TO DIE
  return 0;
}
unsigned int recursive(int m, int n, int k, int * ibegin, int * iend, FILE * file[])
{
  int i = 0;
  int j = 0;
  unsigned int split[m];
  unsigned int kcount = 0; //how many we have found. 
  unsigned int karray[m]; //hmm...
  struct rec input;
  rtrace("Welcome to hell level %d (you are coming from %d) :: m = %d :: n = %d :: k = %d ::", getpid(), getppid(), m, n, k);
  int tempelem;
  //base cases
  //truncate arrays to their length k. 
  for (i = 0; i < m; i++)
  {
    if (((iend[i] - ibegin[i])+1) > k)
    {
      iend[i] = (ibegin[i] + k-1);
      rtrace("k= %d, ibegin[i] = %d", k, iend[i]);
    }
  }
  //grab largest array. 
  int ibase = endElem(iend, ibegin, m, true); //base is the set we are starting with.
  rtrace("iradix = %d, iend = %d, ibegin = %d", ((iend[ibase] - ibegin[ibase])/2 + ibegin[ibase]), iend[ibase], ibegin[ibase]);
  int iradix = ((iend[ibase] - ibegin[ibase])/2 + ibegin[ibase]);
  fseek(file[ibase], iradix*sizeof(unsigned int), SEEK_SET);
  fread(&input,sizeof(struct rec),1,file[ibase]); 
  rewind(file[ibase]);
  unsigned int radix = (htonl(input.x));//split points
  //binary searc
  rtrace("starting binsearch ibase = %d, radix = %u", ibase, radix); 
  for (i = 0; i < m; i++)
  {
    //if not the largest array
    if ( i == ibase )
    {
      split[i] = iradix;
      rtrace("ibase split[i] = %d", split[i]);
    }
    else
    {
      split[i] = binsearch(ibegin[i], iend[i], radix, file, i);
      rtrace("split[i] = %d", split[i]);
    }
  }
  for (i = 0; i < m; i++)
  {
    rtrace("BIG split[i] = %d", split[i]);
    if (split[i] >=0)
    {
      kcount += (split[i]-ibegin[i] +1);
    }
  }
  if (k == 1)
  {
    tempelem = NON_ELEM;
    for (i = 0; i < m; i++) 
    {
      if (ibegin[i] <= iend[i])
      {

        fseek(file[i], ibegin[i]*sizeof(unsigned int), SEEK_SET);
        fread(&input,sizeof(struct rec),1,file[i]); 
        rewind(file[i]);
        if (tempelem == NON_ELEM)
        {
          tempelem = htonl(input.x);
        }
        else if (tempelem > htonl(input.x))
        {   
          //trace("Shouldnt be here");
          tempelem = htonl(input.x);
        }   
      }   
    }   
    return (((float)(tempelem/1.0)));
  }
  else
  { 
    //ensure k is no-larger than m. at this point. 
    bool toofewelems = true;
    for (i = 0; i < m; i++)
    {
      if ((iend[i] - ibegin[i]) > 0)
      { 
        toofewelems = false;
      }
    }
    //check flag. 
    if (toofewelems == true)
    {
      for (i = 0, j = 0; i < m; i++)
      {
        if (ibegin[i] == iend[i])
        {
          fseek(file[i], iend[i]*sizeof(unsigned int), SEEK_SET);
          fread(&input,sizeof(struct rec),1,file[i]);
          rewind(file[i]);
          karray[j] = htonl(input.x);
          rtrace("added j = %d to karray from i = %d, value is = %.0f",j, i, ((float)(htonl(input.x)/1.0)));
        }
      }
      qsort((void*)karray, j+1, sizeof(karray[0]), int_cmp);
      if (k > j)
      { 
        return -1;// (err("you should never see this. k = %d, j = %d, i = %d", k, j, i));
      } 
      return ((float)(karray[k-1]/1.0));
    }
  }
  if (kcount == k)  
  {
    rtrace("In kcount = k");
    //get the largest elem at the end. 
    tempelem = NON_ELEM;
    for (i = 0; i < m; i++)
    {   
      if (split[i] >= 0)   //(ibegin[i] <= iend[i])//selects all undeadedended
      {   
        fseek(file[i], split[i]*sizeof(unsigned int), SEEK_SET);
        fread(&input,sizeof(struct rec),1,file[i]); 
        rewind(file[i]);
        rtrace("FUCKKK %u, tempelem = %d", htonl(input.x), tempelem);

      }
      if (htonl(input.x) >= tempelem)
      {   
        rtrace("AHHHHHHHHHFUCK YOU");
        tempelem = htonl(input.x);
      }

      //else if (ibegin[i] > iend[i])
      //{   
      //  rtrace("an array is deadened");
      //  //return(rtrace("SUPER NASTY BASE CASE PROBLEMS"));
      //}   
      //else 
      //{   
      //something
      //  rtrace("Noman's land. kcount = 1");
      //}   
    }//end for
    return (((float)(tempelem/1.0)));
  }
  if (kcount > k)
  {
    //loop through, if not dead, change end to the split.
    for (i = 0; i < m; i++) 
    {
      if (split[i] >= 0)
      {
        iend[i] = split[i];
        //return (recursive(m,n,k,ibegin,iend,file));
      }
      else
      {
        rtrace("ibegin is error'd");
      }
    }
    return (recursive(m,n,k,ibegin,iend,file));
  }
  else if (kcount < k)
  {
    for (i = 0; i < m; i++)
    {
      if (split[i] >= 0)
      {
        ibegin[i] = split[i]+1;
        rtrace("Kcount = %d", kcount);
        //return (recursive(m,n,(k-kcount),ibegin,iend,file));
      }
    }
    return (recursive(m,n,(k-kcount),ibegin,iend,file));
  }
  rtrace("kcount = %d", kcount);
  exit(1);//(rtrace("WE DID NOT SUCCEED...sad face"));
}//fin
int endElem(int * endArray, int * begArray, int length, bool big_small) // 0 = smallest 1 = largest
{
  int i = 0;
  int elem = NON_ELEM;
  int telem;
  int ielem = NON_ELEM;
  if (big_small == false)
  {
    trace("Endarray iend = %d", endArray[0]);
    //return smallest
    for (i = 0; i < length; i++)
    {
      telem = ((endArray[i] - begArray[i]));
      if (telem < elem) 
      {
        ielem = i;
        elem = telem;
      }
    }
    return ielem;
  }
  else 
  {
    //trace("Endarray iend = %d", endArray[0]);
    for (i = 0; i < length; i++)
    {
      telem = ((endArray[i] - begArray[i]));
      if (telem > elem)
      {
        ielem = i;
        elem = telem;
      }
    }
    //trace("Endarray iend = %d", endArray[0]);
    return ielem;
  }
  err("Something is wrong with simple stuff");
  return 1;
}
int int_cmp(const void *a, const void *b) 
{ 
  const int *ia = (const int *)a; // casting pointer types 
  const int *ib = (const int *)b;
  return *ia  - *ib; 
  /* integer comparison: returns negative if b > a 
     and positive if a > b */ 
}
int binsearch(int ibegin, int iend, unsigned int radix, FILE * file[], int m)
{
  struct rec input;
  int index;
  int temp;
  if (ibegin == iend)
  {
    trace("ibegin == iend");
    fseek(file[m], iend*sizeof(unsigned int), SEEK_SET);
    fread(&input,sizeof(struct rec),1,file[m]); 
    rewind(file[m]);
    if (htonl(input.x) <= radix)
    {
      return iend;
    }
    else
    { 
      return (-2);
    }
  }
  else if (ibegin > iend)
  {
    rtrace("supa dupa error");
    return -1; 
  }
  else
  {
    index = (iend - ibegin)/2 + ibegin;
    fseek(file[m], index*sizeof(unsigned int), SEEK_SET);
    fread(&input,sizeof(struct rec),1,file[m]);
    rewind(file[m]);
    if (htonl(input.x) > radix)
    {
      //trace("lefting ibegin = %d, index = %d, radix = %u, m = %d", ibegin, index-1, radix, m);
      temp = (binsearch(ibegin, index-1, radix, file, m));
      index = -2;
      if (temp == -2)
      {
        return index; 
      }
      else 
      {
        return temp;
      }
    }
    else 
    {
      //trace("righting index = %d, iend = %d, radix = %u, m = %d", index+1, iend, radix, m);

      temp = (binsearch(index+1, iend, radix, file, m));

      if (temp == -2)
      {
        return index;
      }
      else 
      {
        return temp;
      }
    }
  }
}
