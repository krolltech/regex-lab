#include<stdio.h>
#include<string.h>
#include<ctype.h>
#include<stdlib.h>
#include<assert.h>
#include<vector>
#include<iostream>
#include<string>
#include<cstring>
#include<set>


using std::string;
using std::vector;
using std::cout;
using std::endl;
using std::set;

typedef unsigned int uint_t;

/* pair 存储节点的转移关系.*/
struct regex_pair
{
  int c;
  struct regex *node;
};

#define REGEX_PAIR_INIT(pair,_c,_node)  do{        \
    (pair)->c=_c;                                \
    (pair)->node=_node;                                \
  }while(0)

/*分配一打*/
struct regex_pair *regex_pair_malloc_array(uint_t len)
{
  return ( struct regex_pair *)malloc(  sizeof(struct regex_pair) *len );
}

struct regex_pair *regex_pair_clone(struct regex_pair *pair)
{
  struct regex_pair *clone=(struct regex_pair *)malloc(  sizeof(struct regex_pair) );
  if(clone==NULL){
    return NULL;
  }
  memcpy(clone,pair,sizeof(struct regex_pair) );
  return clone;
}

void regex_pair_free_array(struct regex_pair *pairs)
{
  assert(pairs!=NULL);
  free(pairs);
}

struct regex_pair *regex_pair_malloc(int c, struct regex *node)
{
  struct regex_pair *pair=(struct regex_pair *)malloc(sizeof( struct regex_pair));
  assert(node!=NULL);
  if(pair==NULL){
    return NULL;
  }
  pair->c=c;
  pair->node=node;
  return pair;
}
#define REGEX_CAN_FREE 1
#define REGEX_NOT_FREE 0

struct regex
{
  int id;
  /*记录可匹配的路径*/
  struct regex_pair *pair;
  uint_t pair_len;
  char cfree;
};

#define RS_BEGIN  0
#define RS_END   -1

void regex_pair_show(struct regex *r)
{
  uint_t seq=0;
  struct regex_pair *pair;
  assert(r!=NULL);
  cout<<"r:"<<r<<endl;
  while(seq< r->pair_len){
    pair=&r->pair[seq];
    cout<<"c:"<<pair->c<<" to "<<pair->node<<" id:"<< pair->node->id <<endl;
    seq++;
  }
}


struct regex regex_end={
  RS_END,
  NULL,
  0,
  REGEX_NOT_FREE
};

#define REGEX_INIT_BY_ID(regex , _id)  do {        \
    regex->id=_id;                                \
    regex->pair=NULL;                                \
    regex->pair_len=0;                                \
  }while(0)


/*分配一打状态*/
struct regex*regex_malloc_array(uint_t size)
{
  struct regex *r=(struct regex*)calloc(1, size * sizeof(struct regex)  );
  if(r==NULL){
    return NULL;
  }
  r[0].cfree=REGEX_CAN_FREE;
  return r;
}

void regex_free_array(struct regex* regex)
{
  assert(regex!=NULL);
  free(regex);
}

#define RA_BEFORE 0
#define RA_AFTER 1

/*向当前节点@(regex)增加一条边,并保留原来的边*/
int regex_pair_add(struct regex *regex ,const int c, struct regex *node ,int pos)
{
  struct regex_pair *pair_array;
  int offset;
  pair_array=(struct regex_pair*)malloc( sizeof(struct regex_pair) * (regex->pair_len+1 ));
  if(pair_array==NULL){
    return -1;
  }

  if(pos==RA_BEFORE){
    offset=1;
  }else{
    offset=0;
  }

  if(regex->pair_len >0 ){
    memcpy(&pair_array[offset],regex->pair,sizeof(struct regex_pair) * regex->pair_len );
    regex_pair_free_array(regex->pair);
    if(offset ==1 ){
      //添加到第一个
      REGEX_PAIR_INIT(&pair_array[0] ,c,node);
    }else{
      //添加大最后一个
      REGEX_PAIR_INIT(&pair_array[regex->pair_len] ,c,node);
    }
  }else{
    REGEX_PAIR_INIT(&pair_array[0] ,c,node);
  }

  regex->pair=pair_array;
  regex->pair_len=regex->pair_len+1;

  return 0;
}

//所有新边在原来边的后面
#define regex_pair_append(regex, c, node)   regex_pair_add(regex,c,node ,RA_AFTER)

struct regex *regex_malloc(uint_t id)
{
  struct regex *node=(struct regex *)malloc(sizeof(struct regex));
  assert(node!=NULL);
  if(node==NULL){
    return NULL;
  }
  node->id=id;
  node->pair=NULL;
  node->pair_len=0;
  node->cfree=REGEX_CAN_FREE;

  return node;
}

void regex_free(struct regex *regex)
{
  assert(regex!=NULL);
  if(regex->cfree==REGEX_CAN_FREE){
    free(regex);
    if(regex->pair!=NULL){
      free(regex->pair);
    }
  }
}

/*链接两个状态节点.*/
int regex_trace_add(struct regex *from_regex ,const char *c,  struct regex * to_regex)
{
  struct regex_pair *pair=NULL;
  assert(from_regex!=NULL && c!=NULL && to_regex!=NULL);
  if(from_regex==NULL || c==NULL ||to_regex==NULL){
    return -1;
  }

  pair=regex_pair_malloc(*c,to_regex);
  if(pair==NULL){
    return -1;
  }
  assert(from_regex->pair==NULL);
  from_regex->pair=pair;
  from_regex->pair_len=1;

  return 0;
}

/*特殊路径.*/

//空边
#define T_EPS        999

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



int __regex_node_id=0;
int  regex_node_id()
{
  ++__regex_node_id;
  return __regex_node_id;
}


#define RT_SPECIAL 0
#define RT_FLAT  1

/*根据一个特殊的字符建立一个状态*/
struct regex*regex_simple(const char *c ,int type)
{
  struct regex *start;
  struct regex *end;
  struct regex_pair *pair;
  uint_t spec;

  start=regex_malloc(RS_BEGIN);
  if(start==NULL){
    return NULL;
  }

  end=regex_malloc(RS_END);

  if(end==NULL){
    regex_free(start);
    return NULL;
  }

  end->pair_len=0;
  end->pair=NULL;

  if(type==RT_SPECIAL){
    switch(*c){
    case 'a':
      spec=T_ALPHA;
      break;
    case 'd':
      spec=T_DIGIT;
      break;
    case 'w':
      spec=T_ALNUM;
      break;
    case 's':
      spec=T_SPACE;
      break;
    case 'h':
      spec=T_HEX;
      break;
    case 'u':
      spec=T_UPPER;
      break;
    case 'l':
      spec=T_LOWER;
      break;
    case '.':
      spec=T_EVERYTHING;
      break;
    case 'W':
      spec=T_N_ALNUM;
      break;
    case 'S':
      spec=T_N_SPACE;
      break;
    case 'H':
      spec=T_N_HEX;
      break;
    case 'D':
      spec=T_N_DIGIT;
      break;
    case 'A':
      spec=T_N_ALPHA;
      break;
    default:
      assert(0);
    }

  }else{
    spec=*c;
  }

  pair=regex_pair_malloc(spec,end);
  if(pair==NULL){
    regex_free(start);
    regex_free(end);
    return NULL;
  }

  start->pair=pair;
  start->pair_len=1;

  return start;
}

#define regex_special(x)   regex_simple(x,RT_SPECIAL)


/*把一平凡字符串转变为自动机*/
struct regex *regex_flat(const char *patter,uint_t len)
{
  int rtn;
  uint_t seq=0;
  const char *c;
  struct regex_pair *pair;
  struct regex_pair *pair_array;
  struct regex *regex_array;
  struct regex *node;
  struct regex *regex;

  assert(patter!=NULL);
  c=patter;
  assert(*c!='\0');
  if(*c=='\0'){
    return NULL;
  }
  regex_array=regex_malloc_array(len+1);
  if(regex_array==NULL){
    return NULL;
  }

  pair_array=regex_pair_malloc_array(len);
  if(pair_array==NULL){
    regex_free_array(regex_array);
    return NULL;
  }

  regex=&regex_array[0];
  REGEX_INIT_BY_ID( regex , RS_BEGIN );
  seq=1;
  while(*c!='\0'){
    node=&regex_array[seq];
    REGEX_INIT_BY_ID(node, regex_node_id() );
    pair=&pair_array[seq-1];
    REGEX_PAIR_INIT(pair,*c,node);
    regex->pair=pair;
    regex->pair_len=1;
    regex=node;
    ++c;
    seq++;
  }

  //regex->id=RS_END;
  //  cout<<(char)regex_array[len-1].pair->c<<"     ---"<<endl;
  regex_array[len-1].pair=regex_pair_clone(  regex_array[len-1].pair   );
  REGEX_INIT_BY_ID(regex,RS_END);
  return &regex_array[0];
}

//生成[abcdef]的匹配集合.
struct regex *regex_or(const char *pattern, uint_t  len )
{
  const char* c;
  struct regex *begin=regex_malloc(RS_BEGIN);
  struct regex *end=regex_malloc(RS_END);
  struct regex_pair *pair_array;
  struct regex_pair *pair;
  uint_t idx;
  assert(pattern!=NULL && len>0);
  pair_array=regex_pair_malloc_array(len);
  if(pair_array==NULL){
    return NULL;
  }
  cout<<"regex_or:"<<endl;
  c=pattern;
  idx=0;
  while(idx<len){
    pair=&pair_array[idx];
    cout<<*c;
    REGEX_PAIR_INIT(pair,*c,end);
    ++c;
    ++idx;
  }
  cout<<endl;

  begin->pair=pair_array;
  begin->pair_len=len;
  return begin;
}

struct regex* _regex_find_end(struct regex *r, set<struct regex *> *find_set)
{
  struct regex *end=NULL;
  struct regex_pair *pair;
  uint_t seq=0;

  cout<<"find end loop"<<endl;
  assert(r!=NULL);

  if(r->id==RS_END){
    return r;
  }

  while(seq < r-> pair_len){
    pair=&r->pair[seq];
    cout<<"pair"<<endl;
    if(pair->node->id ==RS_END ){
      end=pair->node;
      break;
    }else{
      if(find_set->find(pair->node)==find_set->end() ){
        find_set->insert(pair->node);
        return _regex_find_end(pair->node,find_set);
      }
    }
    seq++;
  }
  assert(end->id==RS_END);
  return end;
}



/* 查找到一个regex的end节点指针.
 * 按深度优先，记录已经遍历过的节点.
 */
struct regex * __regex_find_end(struct regex *r)
{
  set<struct regex *> a_set;
  return _regex_find_end(r,&a_set);
}


/*....存在环.

  将r中所有指向end状态的指针指向新开头.
  @(v).记录所有指向end的指针.
  @(end).记录end指针.
*/
void  __regex_resign_end(struct regex *r ,vector<struct regex **> *v ,struct regex **end)
{
  uint_t seq=0;
  struct regex_pair *pair;
  assert(r!=NULL && v!=NULL && end!=NULL);
  pair=r->pair;
  while(seq< r->pair_len ){
    pair= &r->pair[seq];
    cout<<"pair c:"<<pair->c<<"\tnode:"<<pair->node<<"\tid:"<<pair->node->id<<endl;
    assert(pair->node!=NULL);
    if(pair->node->id==RS_END){
      *end=pair->node;
      //cout<<"pair c:"<<pair->c<<endl;
      v->push_back( &pair->node );
    }else if(pair->node->id!=RS_BEGIN && pair->node!= r){
      __regex_resign_end(pair->node,v,end);
    }
    seq++;
  }
}


void regex_resign_end(struct regex* r,struct regex *n_s)
{
  struct regex *end=NULL;
  vector<struct regex **> v;
  vector<struct regex **>::iterator iter;
  assert(r!=NULL && n_s!=NULL);
  assert(r->id==RS_BEGIN);

  __regex_resign_end(r,&v,&end);

  cout<<"old end:"<<end<<"\tnew end:"<<n_s<<endl;
  assert(end!=NULL);
  regex_free(end);

  iter=v.begin();
  while(iter!=v.end()){
    **iter=n_s;
    //    cout<<"**iter:"<<**iter<<endl;
    iter++;
  }
  //cout<<"over"<<endl;
}

void regex_dps_show(struct regex *regex);


/*
 *串联两个正则表达式
 */
struct regex *regex_join(struct regex *r1,struct regex *r2)
{
  struct regex *r1_end;
  struct regex_pair *pair;
  assert(r1!=NULL );
  assert(r1->id==RS_BEGIN );
  if(r2==NULL){
    return r1;
  }

  cout<<"r1:"<<r1<<" r2"<<r2<<endl;
  cout<<"----"<<endl;
  r1_end=__regex_find_end(r1);
  cout<<"r1_end:"<<r1_end<<endl;
  r1_end->id=regex_node_id();
  pair=regex_pair_malloc(T_EPS,r2);
  r1_end->pair=pair;
  r1_end->pair_len=1;
  r2->id=regex_node_id();

  //regex_dps_show(r1);

  /*
  assert(r1_end->pair_len==0);
  r1_end->pair=r2->pair;
  r1_end->pair_len=r2->pair_len;
  r2->pair_len=0;
  regex_free(r2);
  r1_end->id=regex_node_id();
  cout<<"---"<<endl;
  regex_dps_show(r1);
  */
  // cout<<"resign"<<endl;
  // regex_resign_end(r1,r2);
  // cout<<"new_id"<<endl;
  // r2->id=regex_node_id();
  return r1;
}


/*
 *并联两个正则表达式
 *将r2.start->pair整合到r1.start->pair，并且可以释放掉r2.start.
 *将r2中所有指向r2.end的指针修改为指向r1.end ,释放r2.end.
 */
struct regex* regex_union(struct regex *r1,struct regex *r2)
{
  struct regex_pair *pair_array,*pair;
  struct regex *find;
  struct regex *r1_end=NULL,*r2_end;
  int len;
  int seq1,seq2,tail,head,seq;

  if(r2==NULL){
    return r1;
  }

  assert(r1!=NULL );
  len= r1->pair_len + r2->pair_len ;
  pair_array=regex_pair_malloc_array( len );
  if(pair_array==NULL){
    return NULL;
  }

  // cout<<"r1:"<<r1<<"\tr2:"<<r2<<endl;
  r1_end=__regex_find_end(r1);
  r2_end=__regex_find_end(r2);

  /*需要把eps边拷贝到最后.不定式:eps边永远在状态分支的最后出现.*/
  seq1=r1->pair_len-1;
  seq2=r2->pair_len-1;
  tail=len-1;
  head=0;
  while(seq1>=0 || seq2>=0){
    if(seq1>=0){
      pair=r1->pair+(r1->pair_len-1) - seq1;
    }else{
      pair=r2->pair+(r2->pair_len-1) - seq2;
    }

    if(pair->c==T_EPS){
      assert(tail>=0);
      memcpy(pair_array+tail, pair, sizeof(struct regex_pair));
      pair=pair_array+tail;
      --tail;
    }else{
      memcpy(pair_array+head ,pair, sizeof(struct regex_pair));
      assert(head<len);
      pair=pair_array+head;
      ++head;
    }

    if(pair->node==r2){
      //cout<<"c"<< pair->c <<"\tr1:"<<r1<<endl;
      pair->node=r1;
    }else if(pair->node==r2_end){
      pair->node=r1_end;
    }

    if(seq1>=0){
      --seq1;
    }else{
      --seq2;
    }
  }

  assert(seq1==-1 && seq2==-1);

  //  cout<<"r1 end:"<<r1_end<<endl;
  assert(r1_end!=NULL);
  regex_resign_end(r2,r1_end);
  //cout<<"resign end"<<endl;
  regex_free(r2);
  regex_pair_free_array(r1->pair);
  r1->pair=pair_array;
  r1->pair_len=len;

  //regex_pair_show(r1);
  return r1;
}

//实现*或+重复
#define RL_STAR  0
#define RL_PLUS  1

//实现贪婪匹配或惰性匹配
#define RM_GREEDY  0
#define RM_LAZY   1


/*查找所有执行end的节点以及其边.以pair返回*/
void __regex_find_node_to_end(struct regex *regex,vector<struct regex_pair> *v)
{
  struct regex_pair *pair;
  uint_t seq=0;
  assert(regex!=NULL);

  while(seq < regex->pair_len){
    struct regex_pair p;
    pair=&regex->pair[seq];
    if(pair->node->id==RS_END){
      p.c=pair->c;
      p.node=regex;
      v->push_back(p);
    }else{
      __regex_find_node_to_end(pair->node,v);
    }
    ++seq;
  }
}

/*
 *让正则自动具有重复功能
 *@(loop_m):[ RL_START | RL_PLUS]
 *@(match_m):[RM_GREEDY || RM_LAZY]
 */
void regex_loop(struct regex *regex, int loop_m ,int match_m)
{
  int pos;
  struct regex *end;
  vector<struct regex_pair> v;
  vector<struct regex_pair>::iterator iter;

  __regex_find_node_to_end(regex,&v);
  end=__regex_find_end(regex);

  if(match_m==RM_GREEDY){
    pos=RA_BEFORE;
  }else{
    pos=RA_AFTER;
  }

  iter=v.begin();
  while(iter!=v.end()){
    regex_pair_add(iter->node,iter->c, regex ,pos);
    ++iter;
  }

  if(loop_m==RL_STAR){
    regex_pair_append(regex,T_EPS,end);
  }
}


/*进行状态匹配.*/
int __regex_match(struct regex *start,struct regex *regex , const char *c,string *result)
{
  bool match=false;
  struct regex_pair *pair;
  uint_t seq=0;
  assert(regex!=NULL &&c!=NULL);

  cout<<"result:"<<*result<<" id:"<<regex->id<<" regex:"<<regex<<" meet:"<<*c <<endl;

  if(regex->id==RS_END){
    if(result->length()>0){
      cout<<"======> match:\""<<*result<<"\""<<endl;
    }
    return 0;
  }

  if(*c=='\0'){
    return -1;
  }

  while(seq< regex->pair_len ){
    pair=&regex->pair[seq];

    switch(pair->c){
    case T_EPS:
      //      cout<<"match eps"<<" to "<<pair->node<<endl;
      match=true;
      break;
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
      match= (pair->c==*c );
    }

    if(match==false){
      ++seq;
      continue;
    }

    /*非eps，吃掉字符.*/
    if(pair->c!=T_EPS){
      result->append(1,*c);
      ++c;
    }

    if(__regex_match(start,pair->node,c,result) ==0){
      return 0;
    }
    /*非eps，回退字符.*/
    if(pair->c!=T_EPS){
      result->erase(result->length() -1);
      --c;
    }

    ++seq;
  }

  return -1;
}

/*进行自动机匹配*/
int regex_match(struct regex *regex, const char *c)
{
  string result;
  cout<<"begin regex:"<<regex<<endl;
  while(*c!='\0'){
    result="";
    if(__regex_match(regex,regex,c,&result) ==0){
      c+= result.length();
      if(result.length()==0){
        c++;
      }
    }else{
      c++;
    }
  }
  return 0;
}




const char *c;

/*根据pattern建立自动机*/
struct regex* regex_make()
{
  struct regex *r=NULL;
  struct regex *t=NULL;
  const char *c1;
  bool join=true;
  bool can_eat_pair=false;
  bool need_return=false;

  cout<<"regex_make:"<<c<<endl;
  if(*c=='\0'){
    return NULL;
  }
  while(*c!='\0'){
    cout<<"meet:"<<*c<<endl;
    switch(*c){
    case '|':
      ++c;
      t=regex_make();
      join=false;
      need_return=true;
      //++c;
      break;
    case '\\':
      ++c;
      t=regex_simple(c,RT_SPECIAL);
      ++c;
      break;
    case '(':
      ++c;
      can_eat_pair=true;
      t=regex_make();
      cout<<") over c:"<<*c<<endl;
    case ')':
      cout<<") over"<<endl;
      if(can_eat_pair==true){
        cout<<"eat:"<<*c<<endl;
        ++c;
      }else{
        t=NULL;
        need_return=true;
      }
      break;
    case '[':
      ++c;
      c1=c;
      while(*c!=']'){
        ++c;
      }
      t=regex_or(c1,c-c1 );
      ++c;
      break;
    case '.':
      t=regex_simple(c,RT_SPECIAL);
      ++c;
      break;
    default:
      t=regex_simple(c ,RT_FLAT);
      ++c;
      break;
    }

    //向前看一个字符，决定是否*或?或+.
    switch(*c){
     case '*':
      cout<<"pre see: *"<<endl;
      ++c;
      if(*c=='?'){
        regex_loop(t,RL_STAR,RM_LAZY);
        ++c;
      }else{
        cout<<"make loop"<<endl;
        regex_loop(t,RL_STAR,RM_GREEDY);
      }
      break ;
    case '+':
      ++c;
      if(*c=='?'){
        regex_loop(t,RL_PLUS,RM_LAZY);
        ++c;
      }else{
        regex_loop(t,RL_PLUS,RM_GREEDY);
      }
      cout<<"pre see:"<<*(c-1)<<endl;
      break ;
    default:
      break;
    }

    if(r==NULL){
      r=t;
    }else{
      if(join==true){
        cout<<"join"<<endl;
        r=regex_join(r,t);
        cout<<"join over"<<endl;
      }else{
        cout<<"union "<<(char)r->pair[0].c<<" "<< (char)t->pair[0].c<<endl;
        r=regex_union(r,t);
        cout<<"regex_make over c:"<<*c<<endl;
        return r;
      }
    }

    if(need_return==true){
      cout<<"regex_make over c:"<<*c<<endl;
      return r;
    }
  }
  cout<<"regex_make over c:"<<*c<<endl;
  return r;
}


void regex_dps_show(struct regex *regex)
{
  uint_t seq=0;
  struct regex_pair *pair;
  assert(regex!=NULL);
  /*
    if(regex->id ==RS_BEGIN){
    cout<<"start"<<endl;
    }
  */
  cout<<"\t"<<regex<<endl;
  cout<<"\trid:"<<regex->id<<endl;
  if(regex->id==RS_END){
    return ;
  }

  while(seq< regex->pair_len ){
    pair=&regex->pair[seq];
    printf("%c -> \n",(char) pair->c );
    regex_dps_show(pair->node);
    ++seq;
  }
}


void regex_bps_show(struct regex *regex)
{
  uint_t seq=0;
  struct regex_pair *pair;
  assert(regex!=NULL);

  printf("rid:%d \n",regex->id);
  if(regex->id==RS_END){
    return ;
  }

  while(seq< regex->pair_len ){
    pair=&regex->pair[seq];
    printf("%c \n",(char) pair->c );
    ++seq;
  }
  seq=0;
  while(seq< regex->pair_len ){
    pair=&regex->pair[seq];
    regex_dps_show(pair->node);
    ++seq;
  }
}


//测试特殊转义.
void test_regex_special(const char *c)
{
  struct regex *r1=regex_special("w");
  struct regex *r2=regex_special("d");
  struct regex *first=regex_flat("abc",3);
  struct regex *tail=regex_flat("efg",3);
  struct regex *tmp,*r3,*r4,*r5;
  cout<<"\\w "<<endl;
  regex_match(r1,c);
  cout<<"\\d "<<endl;
  regex_match(r2,c);
  tmp=regex_join(r1,r2);
  cout<<"\\w\\d"<<endl;
  regex_match(tmp,c);
  cout<<"abc\\wefg"<<endl;
  r3=regex_join(first,r1);
  r4=regex_join(r3,tail);
  regex_match(r4,c);

}


//测试loop

void test_regex_loop(const char *c)
{
  struct regex *simple=regex_flat("b",1);
  struct regex *more_d=regex_special("d");

  regex_loop(more_d,RL_STAR,RM_GREEDY);
  regex_loop(simple,RL_PLUS,RM_GREEDY);

  cout<<"simple..."<<endl;
  regex_match(simple,c);

  cout<<"more..."<<endl;
  regex_match(more_d,c);
}

void test_regex_loop_2(const char *c)
{
  struct regex *simple=regex_flat("b",1);
  struct regex *more_d=regex_special("d");
  struct regex *r=regex_union(simple,more_d);
  regex_loop(r,RL_STAR,RM_GREEDY);
  regex_pair_show(r);
  regex_match(r,c);
}

//测试start
void test_regex_star(const char *c)
{
  struct regex *start=regex_malloc(RS_BEGIN);
  struct regex *r1=regex_malloc(1);
  struct regex *end=regex_malloc(RS_END);


  regex_pair_append(start,'b',r1);
  regex_pair_append(r1,'a',r1);
  regex_pair_append(r1,'a',end);
  regex_pair_append(r1,T_EPS,end);
  regex_match(start,c);

}

//测试并联
void test_regex_union()
{
  struct regex *r1 = regex_flat("hello",5);
  struct regex *r2 = regex_flat("world",5);
  struct regex *r3;

  regex_dps_show(r1);
  regex_dps_show(r2);

  cout<<"regex:  (hello|world)"<<endl;
  r3=regex_union(r1,r2);
  regex_dps_show(r3);
}

void test_regex_union_2(const char *c)
{
  struct regex *r1=regex_flat("m",1);
  struct regex *r2=regex_special("d");
  struct regex *r3;
  regex_loop(r1,RL_STAR , RM_GREEDY);
  regex_loop(r2,RL_STAR , RM_GREEDY);

  //  regex_match(r1,"mmmmm23m23mmmmm");
  // regex_match(r2,"232323fefw2323");

  cout<<"union r1 and r2"<<endl;
  r3=regex_union(r1,r2);
  cout<<"union ok"<<endl;
  regex_match(r3,c);
}

void test_regex_union_3(const char *c)
{
  struct regex *r1=regex_flat("m",1);
  struct regex *r2=regex_special("d");
  struct regex *r3;
  regex_loop(r1,RL_STAR , RM_GREEDY);
  regex_loop(r2,RL_STAR , RM_LAZY);

  //  regex_match(r1,"mmmmm23m23mmmmm");
  // regex_match(r2,"232323fefw2323");

  cout<<"union r1 and r2"<<endl;
  r3=regex_union(r1,r2);
  cout<<"union ok"<<endl;
  regex_match(r3,c);
}


//测试声称串联
int test_regex_join()
{
  struct regex *r1 ;
  struct regex *r2 ;
  struct regex *r3 ;

  /*
    r1 = regex_flat("hello",5);
    r2 = regex_flat("world",5);

    regex_dps_show(r1);
    regex_dps_show(r2);

    r3=regex_join(r1,r2);
    printf("dps_show\n");
    regex_dps_show(r3);
  */

  r1=regex_flat("h",1);
  r2=regex_flat("w",1);
  r3=regex_join(r1,r2);

  regex_dps_show(r3);

  return 0;
}


int test_regex_or()
{
  struct regex *r=regex_or("abcdef",6);
  regex_bps_show(r);
  return 0;
}

void test_regex_or_and_join()
{
  struct regex *r1=regex_or("hello",5);
  struct regex *r2=regex_flat("world",5);
  struct regex *r3=regex_or("ab",2);
  struct regex *r4=regex_join(r1,r2);
  struct regex *r5;

  printf("regex: [hello]world[ab] \n");

  r5=regex_join(r4,r3);
  regex_dps_show(r5);
}

void test_match(const char *c)
{
  struct regex *r1 =regex_flat("hello",5);
  struct regex *r2=regex_or("world",5);
  struct regex *r3=regex_join(r1,r2);
  regex_match(r3,c);
}



int test_main(int argc , char *argv[])
{
  test_regex_loop_2(argv[1]);
  //  test_regex_union_3(argv[1]);
  //  test_regex_loop(argv[1]);
  //  test_regex_star(argv[1]);
  //  test_regex_special(argv[1]);
  //  test_regex_join();
  //test_regex_or_and_join();
  //test_regex_union();
  //  test_match();
  //  test_match(argv[1]);
  return 0;
}


int main(int argc , char *argv[])
{
  struct regex *r;
  c=argv[1];
  r=regex_make();
  cout<<"-->"<<endl;
  //regex_pair_show(r->pair[0].node);
  //  regex_dps_show(r);
  cout<<"----->>"<<endl;

  if(argv[2]!=NULL){
    regex_match(r,argv[2]);
  }else{
    regex_match(r,"");
    cout<<"null"<<endl;
  }
  return 0;
}
