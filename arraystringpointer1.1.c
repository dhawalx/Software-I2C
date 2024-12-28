/******************************************************************************

Welcome to GDB Online.
GDB online is an online compiler and debugger tool for C, C++, Python, Java, PHP, Ruby, Perl,
C#, OCaml, VB, Swift, Pascal, Fortran, Haskell, Objective-C, Assembly, HTML, CSS, JS, SQLite, Prolog.
Code, Compile, Run and Debug online from anywhere in world.

*******************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
struct sentence{
    char* mystr;
    int strcount;
};
struct paragraph{
    struct sentence *mySen;
    int sent_cnt;
};

struct document{
    struct paragraph *myPara;
    int para_cnt;
};

int main()
{
    char* arrstr = "1First.1Second. \n 2First.2Second. \n 3Third Para.3Second String\n";
    printf("arrstr-%s\n", arrstr);
    int sent_cnt = 0;
    int paracount = 0;
    for(int i=0; i<strlen(arrstr)+1; i++)
    {
        if(arrstr[i]== '.')
        {
            sent_cnt++;
        }
        if(arrstr[i]=='\n' || arrstr[i]=='\0'  )
        {
            paracount++;
        }
    }
    printf("sent_cnt %d, paracount %d\n", sent_cnt, paracount);
    
    struct document *myDoc;
    myDoc = (struct document *)malloc(sizeof(struct document));
    myDoc->para_cnt = paracount;
    printf("myDoc->para_cnt %d\n", myDoc->para_cnt );
    
    myDoc->myPara = (struct paragraph*)malloc( sizeof(struct paragraph)*(myDoc->para_cnt) );
    
    volatile int sendindex =0;
    for(int p =0; p< myDoc->para_cnt; p++)
    {
        int sent_cnt_lcl =0;
        printf("--p=%d-sendindex--- %d\n", p, sendindex);
      
        while( arrstr[sendindex] == ' ' || arrstr[sendindex] == '\n' || arrstr[sendindex] == '.' )
        { 
            sendindex++;
            printf("sendindex++ %d", sendindex );
        }
        for(int i=sendindex; i<strlen(arrstr)+1; i++)
        {
            if(arrstr[i]== '.' || arrstr[i]=='\0')
            {
                sent_cnt_lcl++;
            }
            if(arrstr[i]=='\n' )
            {
                break;
            }
        } 
        printf("sent_cnt_lcl %d\n", sent_cnt_lcl);
        myDoc->myPara[p].sent_cnt = sent_cnt_lcl;
        myDoc->myPara[p].mySen = (struct sentence*)malloc( sizeof(struct sentence)*(myDoc->myPara[p].sent_cnt));
        
        for (int k =0; k< myDoc->myPara[p].sent_cnt; k++ )
        {
            printf("k %d\n", k);
            for(int j=sendindex; j<strlen(arrstr)+1; j++)
            {
                if( arrstr[j] == '.' || arrstr[j]=='\0' )
                {
                   printf("j %d  sendindex %d\n", j, sendindex );    
                   myDoc->myPara[p].mySen[k].mystr =  (char*)malloc( (j-sendindex)*sizeof(char));
                   strncpy(myDoc->myPara[p].mySen[k].mystr, arrstr+sendindex, j-sendindex);
                   j++;
                   sendindex = j;
                   printf("sendindex %d\n", sendindex);
                   printf("myDoc->myPara[%d].mySen[%d].mystr= %s\n",p, k, myDoc->myPara[p].mySen[k].mystr);
                   break;
                }
            } 
            //printf("scount %d\n", scount);
        } //sent_cnt
    }
    return 0;
}
