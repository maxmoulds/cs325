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

#define NON_ELEM -1

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

int recursive(int n, int m, int k, int * ibegin, int * iend, FILE * file[]);
int endElem(int * endArray, int * begArray, int length, bool big_small);

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
    iend[i] = (n-1); //could be off by 1
  }
  //trace("testing ith of beg and end ::beg %d ::end %d",ibegin[0], iend[m-1]);
  //stuff for this driver
  struct timeval  tv1, tv2;
  gettimeofday(&tv1, NULL);
  /* Do sutff */
  trace("Entering Hell (%d) :: m = %d :: n = %d :: k = %d ::", getpid(), m, n, k);
  trace("The k-th smallest element is :: %d ::", recursive(m,n,k,ibegin,iend,file));
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
int recursive(int m, int n, int k, int * ibegin, int * iend, FILE * file[])
{
  int i = 0;
  int j = 0;
  int kcount = 0; //how many we have found. 
  int karray[m]; //hmm...
  struct rec input;
  rtrace("Welcome to hell level %d (you are coming from %d) :: m = %d :: n = %d :: k = %d ::", getpid(), getppid(), m, n, k);
  int ibase = endElem(iend, ibegin, m, true); //base is the set we are starting with. 
  int iradix = ((int)((iend[ibase] - ibegin[ibase])/2) + ibegin[ibase]);
  fseek(file[ibase], iradix*sizeof(unsigned int), SEEK_SET);
  fread(&input,sizeof(struct rec),1,file[ibase]); 
  rewind(file[ibase]);
  unsigned int radix = htonl(input.x);
  rtrace("radix = %.0f",((float)(radix/1.0)));
  rtrace("BASE M-SET: Updating k-count :: old = %d :: newINVALID = %d ::", kcount, (iradix-ibegin[ibase]));
  //kcount += (iradix-ibegin[ibase]);
  //bin search. 
  unsigned int tempelem;
  //base cases
  if (k == 1)
  {
    tempelem = NON_ELEM;
    for (i = 0; i < m; i++) 
    {
      fseek(file[i], ibegin[i]*sizeof(unsigned int), SEEK_SET);
      fread(&input,sizeof(struct rec),1,file[i]); 
      rewind(file[i]);
      if (tempelem > htonl(input.x))
      {
        tempelem = htonl(input.x);
      }
    }
    return (rtrace("An answer is = %.0f",((float)(tempelem/1.0))));
  }
  //base case
  else if (kcount == k) 
  {
    //no recurse, i have answer in karray. 
    //use the largest of the m-set iend elements
    for (i = 0; i < m; i++)
    {
      tempelem = NON_ELEM;
      if (iend[i] > ibegin[i])
      {
        fseek(file[i], iend[i]*sizeof(unsigned int), SEEK_SET);
        fread(&input,sizeof(struct rec),1,file[i]); 
        rewind(file[i]);
        if (htonl(input.x) > tempelem)
        {
          tempelem = htonl(input.x);
        }
      }
      else
      {
        rtrace("an array is deadened");
        //return(rtrace("SUPER NASTY BASE CASE PROBLEMS"));
      }
    }
    return (rtrace("base ending k = kcount, answer is = %.0f", ((float)(tempelem/1.0))));
  }
  //all msets below ibase, general case, not a base case below. 
  else
  {
    bool toofewelems = true; 
    for (i = 0; i < m; i++)
    {
      if (i == ibase)
      {
        rtrace("...not doing ibase twice");
      }
      else if ((iend[i] - ibegin[i]) < 0)
      {
        rtrace("SILENCE YOU INSIGNIFICANT M-SET :: %d ::", i);
      }
      else
      {
        //go ahead and binsearch. 
        toofewelems = false;
        rtrace("i is :: %d ::", i);
        bool binfound = false;
        iradix = ((int)((iend[i] - ibegin[i])/2) + ibegin[i]);
        while (binfound == false)
        {
          //get index
          //read
          fseek(file[i], iradix*sizeof(unsigned int), SEEK_SET);
          fread(&input,sizeof(struct rec),1,file[i]);
          rewind(file[i]);
          tempelem = htonl(input.x);
          //check elem. 
          if (tempelem <= radix) 
          {
            //binfound. add to kcount
            //we now need to go right to make sure that the elem is the actual
            //smallest.
            bool smallest = false; 
            int tempradix = ((iend[i] - ibegin[i])/2) + ibegin[i];
            unsigned int itempelem = NON_ELEM;
            while (smallest == false)
            {
              sleep(1);
              //check the right. 
              if (tempradix == iend[i])
              {
                //read the last value
                fseek(file[i], iend[i]*sizeof(unsigned int), SEEK_SET);
                fread(&input,sizeof(struct rec),1,file[i]);
                rewind(file[i]);
                if (htonl(input.x) <= radix)
                {
                  //dont deaden
                  iradix = tempradix;
                  smallest = true;
                  binfound = true;
                  //do something?
                  rtrace("i think we are done binary searching...");
                }
                else
                {
                  smallest = true;
                  binfound = true;
                  //also dont anything. 
                  //deaden array 
                }
              }
              else 
              {
                fseek(file[i], tempradix*sizeof(unsigned int), SEEK_SET);
                fread(&input,sizeof(struct rec),1,file[i]);
                rewind(file[i]);
                rtrace("tempradix elem value = %0.f, i = %d, ibegin = %d, iend = %d, radix = %d,", ((float)(htonl(input.x))), i, ibegin[i], iend[i], radix);
                itempelem = ((float)(htonl(input.x)));
                if (iend[i] == ibegin[i]) 
                {
                  rtrace("here is the probelm");
                }
                else if (itempelem > radix) 
                {
                  //we know that no elements past where we are are candidates
                  iend[i] = tempradix;
                  smallest = true;
                  binfound = true;
                  //go left. 
                }
                else if (itempelem < radix) // (itempelem <= radix)
                {
                  tempradix = ((iend[i] - tempradix)/2) + ibegin[i];
                  //go right
                }
                else
                {
                  //it equals == radix or we are at the end. 
                }
                tempradix = ((iend[i] - tempradix)/2) + tempradix;
              }
            }//while
            rtrace("left :: %x <= %.0f ::", tempelem, ((float)(radix/1.0)));
            // we are done, so update indexes. 
            //iend[i] = iradix; //set the i-th m-set ending element to the current radix index. 
            //update k-statusii
            //rtrace("Updating k-count :: old = %d :: new = %d ::", kcount, (iend[i]-ibegin[i]));
            //kcount += (iend[i]-ibegin[i]);
            binfound = true; //leave to get next m-set (i++)
          }
          else if (tempelem > radix)
          {
            //go right
            if (iradix > 1)
            {
              //binsearch select next elem.
              iradix = (iradix/2);
            }
            else if (iradix == 1)
            {
              // one more elem to check
              iradix = 0;
            }
            else 
            {
              //deadening the array
              if (k <= kcount)
              {
                rtrace("iradix is 0 and this entire mset is not going to contribute... slacker");
                iend[i] = 0;
                ibegin[i] = 1;
                //should i also decrement m.
                //reset radix.
                binfound = true; //because we are giving up on this mset. 
              }
              else if (k > kcount)
              {
                rtrace("keep the top half");
                //iend[i] stays the same
                ibegin[i] = iradix;
              }
              else
              {
                rtrace("Super duper error in binsearch deadening");
              }
            }
            rtrace("right, a (possibly) new iradix is :: %d ::", iradix);
          }//for loop
          //done with binsearch
          else
          {
            rtrace("Something horribly wrong");
          }
        }//end while
      }//end else in for
      //end BINSEARCH
    }//end for
    //
    if (toofewelems == true)
    {
      //that one base case we skipped. 
      return(rtrace("toofewelems - we should have elem :: k = %d :: ibase = %d :: iradix = %d :: radix = %.0f :: kcount = %d :: ", k, ibase, iradix, ((float)(radix/1.0)), kcount));
    }
  }//end k == 1 catch

  //time to kcount. 
  for (i = 0; i < m; i++)
  {
    kcount += (iend[i] - ibegin[i]);
    rtrace("kcount has been updated to %d", kcount);
  }
  //okay now if kcount is ...
  if (kcount > k)
  {
    //left side
    //no need to recurse?...
    //sort karray...
    return (rtrace("...a rabbit hole = %d", recursive(m, n, k, ibegin, iend, file))); 
    //rtrace("i is %d, input.x is %x", i, htonl(input.x));
    //fseek(file[ibase], k*sizeof(unsigned int), SEEK_SET);
    //fread(&input,sizeof(struct rec),1,file[ibase]);
    //rewind(file[ibase]);
    //return (rtrace("I swear we have the answer, its just not ready for the spotlight yet, but i'll try. :: %.0f :: at(i):: %d ::",((float)(htonl(input.x)/1.0)), k));
  }
  else 
  {
    //need to recurse.
    //right side
    return (recursive(m, n,(k-kcount),ibegin, iend, file));
  }
  exit(rtrace("WE DID NOT SUCCEED...sad face"));
}
int endElem(int * endArray, int * begArray, int length, bool big_small) // 0 = smallest 1 = largest
{
  int i = 0;
  int elem = NON_ELEM;
  int telem;
  int ielem = NON_ELEM;
  if (big_small == false)
  {
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
    for (i = 0; i < length; i++)
    {
      telem = ((endArray[i] - begArray[i]));
      if (telem > elem)
      {
        ielem = i;
        elem = telem;
      }
    }
    return ielem;
  }
  err("Something is wrong with simple stuff");
  return 1;
}
