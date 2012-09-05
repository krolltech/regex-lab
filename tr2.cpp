#include<cstdio>
#include<cstdlib>
#include<cstring>
#include<map>
#include<iostream>
#include<cassert>
#include<list>
#include<string>
#include<cstring>
#include<vector>

using std::vector;
using std::list;
using std::map;
using std::pair;
using std::string;
using std::cout;
using std::endl;

#define RSTATUS_BEGIN 0
#define RSTATUS_END -1

#define STATUS_TYPE int

typedef unsigned int uint_t;

struct rnode
{
  STATUS_TYPE status;
  map<int, struct rnode *> trace;
  uint_t match[2];
};


inline void rnode_free(struct rnode *node)
{
  delete node;
}

inline struct rnode *rnode_malloc(STATUS_TYPE status)
{
  struct rnode *node=new struct rnode;
  node->status=status;
  node->match[0]=node->match[1]=0;
  return node;
}

#define STABLE_STATUS_NUM 30

int tail;
string result;

#define T_EVERYTHING 1000
#define T_DIGIT  1002
#define T_LOWER 1003
#define T_UPPER 1004
#define T_ALPHA 1005
#define T_SPACE 1006
#define T_HEX   1007
#define T_ALNUM  1008

#define T_N_DIGIT 1009
#define T_N_HEX   1010
#define T_N_ALPHA 1011
#define T_N_SPACE 1012
#define T_N_ALNUM 1013

int regex_match(struct rnode *regex, const char *c)
{
  cout<<regex->status<<" "<<*c<<endl;

  if(regex->status== RSTATUS_END){
    cout<<"match:"<<result<<endl;
    return 0;
  }

  if(*c=='\0'){
    return -1;
  }

  for(map<int ,struct rnode *>::iterator iter =regex->trace.begin();
      iter!=regex->trace.end() ;++iter){

    bool match;
    switch(iter->first){
    case T_EVERYTHING:
      match=true;
      break;
    case T_N_DIGIT:
      match= ( isdigit(*c)!=1) ;
      break;
    case T_DIGIT:
      match= (isdigit( *c ) ==1) ;
      break;
    case T_N_ALPHA:
      match = (isalpha(*c)!=1 );
      break;
    case T_ALPHA:
      match= (isalpha( *c ) ==1) ;
      break ;
    case T_N_ALNUM:
      match= ( isalnum(*c)!=1);
      break;
    case T_ALNUM:
      match= (isalnum( *c ) ==1) ;
      break ;
    case T_LOWER:
      match =( islower( *c )==1 );
      break ;
    case T_UPPER:
      match = (isupper( *c )==1 );
      break;
    case T_N_SPACE:
      match= (isspace(*c)!=1);
      break;
    case T_SPACE:
      match= (isspace( *c )==1 );
      break;
    case T_N_HEX:
      match= (isxdigit(*c)!=1);
      break;
    case T_HEX:
      match= (isxdigit(*c)==1 );
      break;
    default:
      match= (iter->first==*c );
    }

    if(match==false){
      continue;
    }

    tail++;
    result.append(1,*c);
    c++;

    if( regex_match(iter->second,c)==0) {
      return 0;
    }

    --tail;
    result.erase( tail );
    --c;
  }


  if(regex->status==RSTATUS_BEGIN){
    ++c;
    regex_match(regex,c);
    result="";
  }
  return 1 ;
}

//根据正则表达式建立自动机.
struct rnode * make_automachine(const char *pattern)
{
  int id=1;
  struct rnode *begin=rnode_malloc(RSTATUS_BEGIN);
  struct rnode *regex=begin;
  struct rnode *tmp;

  const char *c=pattern;
  vector<int> cs;

  while(*c!='\0'){
    cout <<"p:"<<*c<<endl;
    switch(*c){
    case '+':
      {
        vector<int>::iterator iter=cs.begin();
        tmp=rnode_malloc(id++);
        while(iter!=cs.end() ){
          regex->trace[*iter]=regex;
          iter++;
        }
      }
      break;
    case '\\':
      c++;
      switch(*c){
      case 'a':
        tmp=rnode_malloc(id++);
        regex->trace[T_ALPHA] = tmp;
        regex=tmp;
        cs=vector<int>(1,T_ALPHA);
        break;
      case 'w':
        tmp=rnode_malloc(id++);
        regex->trace[T_ALNUM] = tmp;
        regex=tmp;
        cs=vector<int>(1,T_ALNUM);
        break;
      case 's':
        tmp=rnode_malloc(id++);
        regex->trace[T_SPACE] =tmp;
        regex=tmp;
        cs=vector<int>(1,T_SPACE);
        break;
      case 'd':
        tmp=rnode_malloc(id++);
        regex->trace[T_DIGIT] =tmp;
        regex=tmp;
        cs=vector<int>(1,T_DIGIT);
        break;
      case 'h':
        tmp=rnode_malloc(id++);
        regex->trace[T_HEX] =tmp;
        regex=tmp;
        cs=vector<int>(1,T_HEX);
        break;
      case 'W':
        tmp=rnode_malloc(id++);
        regex->trace[T_N_ALNUM] =tmp;
        regex=tmp;
        cs=vector<int>(1,T_N_ALNUM);
        break;
      case 'S':
        tmp=rnode_malloc(id++);
        regex->trace[T_N_SPACE] =tmp;
        regex=tmp;
        cs=vector<int>(1,T_N_SPACE);
        break;
      case 'D':
        tmp=rnode_malloc(id++);
        regex->trace[T_N_DIGIT] =tmp;
        regex=tmp;
        cs=vector<int>(1,T_N_DIGIT);
        break;
      case 'H':
        tmp=rnode_malloc(id++);
        regex->trace[T_N_HEX] =tmp;
        regex=tmp;
        cs=vector<int>(1,T_N_HEX);
        break;
      }
      break;
    case '.':
      tmp=rnode_malloc(id++);
      regex->trace[T_EVERYTHING]=tmp;
      regex=tmp;
      cs=vector<int>(1,T_EVERYTHING);
      break;
    case '[':
      ++c;
      tmp=rnode_malloc(id++);
      cs=vector<int>();
      while(*c!=']'){
        regex->trace[*c]=tmp;
        c++;
        cs.push_back(*c);
      }
      regex=tmp;
      break;
    default:
      tmp=rnode_malloc(id++);
      regex->trace[*c]=tmp;
      regex=tmp;
      cs=vector<int>(1,*c);
    }
    c++;
  }

  regex->status=RSTATUS_END;
  return begin;
}


static int id=0;
int regex_id()
{
  ++id;
  return id;
}


/*
 * @pattern:完全无其他字符的pattern.
*/
struct rnode *regex_flat(const char *pattern)
{
  struct rnode *begin=rnode_malloc(RSTATUS_BEGIN);
  struct rnode *regex=begin;
  struct rnode *tmp;
  const char *c=pattern;

  assert(*c!='\0');
  while(*c!='\0'){
    tmp=rnode_malloc( regex_id());
    regex->trace[*c]=tmp;
    regex=tmp;
    ++c;
  }
  regex->status=RSTATUS_END;

  return begin;
}

/*测试.*/
void test_regex_flat(const char *pattern, const char *string)
{
  struct rnode *regex=regex_flat(pattern);
  regex_match(regex,string);
}

/*把两条平坦正则表达式串联*/
struct rnode *regex_join(list<struct rnode *> list)
{
  list<struct rnode *>::iterator iter=list.begin();
  struct rnode* begin=rnode_malloc(RSTATUS_BEGIN);
  while(iter!= list.end()){
    struct rnode *tmp=*iter;
    for(map<int, struct rnode *>::iterator cursor=tmp->trace.begin();
        cursor!=tmp->trace.end(); ++cursor){
      //begin->trace[cursor->first]=
    }
  }

}


int main(int argc,char *argv[])
{
  const char *pattern;
  const char *string;
  if(argc<3){
    cout<<"usage: "<<argv[0]<<" pattern" <<" string" <<endl;
    return 0;
  }

  pattern=argv[1];
  string=argv[2];

  test_regex_flat(pattern,string);

}


int main2(int argc ,char *argv[])
{
  struct rnode *begin=rnode_malloc(RSTATUS_BEGIN);
  struct rnode *end=rnode_malloc(RSTATUS_END);
  struct rnode *table[STABLE_STATUS_NUM+1];
  for(unsigned int i=0; i<=STABLE_STATUS_NUM ;i++){
    table[i]=rnode_malloc(i);
  }

  /* a(bb)+a
  begin->trace['a']=table[1];
  table[1]->trace['b']=table[2];
  table[2]->trace['b']=table[3];
  table[3]->trace['b']=table[2];
  table[3]->trace['a']=end;
  */

  /*a.ba*/
  /*
  begin->trace['a']=table[1];
  table[1]->trace['.']=table[2];
  table[2]->trace['b']=table[3];
  table[3]->trace['a']=end;
  */

  //  /*.+*/
  //  /*
  begin->trace['/']=table[1];
  table[1]->trace['*']=table[2];
  table[2]->trace[T_EVERYTHING]=table[3];
  table[3]->trace['*']=table[4];
  table[3]->trace[T_EVERYTHING]=table[3];
  table[4]->trace['/']=end;
  //  */


  // a[hello|word]day
  /*
  begin->trace['a']=table[1];
  table[1]->trace['h']=table[2];
  table[2]->trace['e']=table[3];
  table[3]->trace['l']=table[4];
  table[4]->trace['l']=table[5];
  table[5]->trace['o']=table[6];
  table[1]->trace['w']=table[7];
  table[7]->trace['o']=table[8];
  table[8]->trace['r']=table[9];
  table[9]->trace['d']=table[10];
  table[10]->trace['d']=table[11];
  table[6]->trace['d']=table[11];
  table[11]->trace['a']=table[12];
  table[12]->trace['y']=end;
    */

  struct rnode *regex=make_automachine(argv[1]);
  regex_match(regex,argv[2]);

  return 0;
}


