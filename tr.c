#include<stdio.h>

#define C_BEGIN  0
#define C_END    1

struct rnode
{
  int status;
  struct rnode *next;
  
}rnode_t;

#define __RNODE_INIT(status,next)  { status, next }

#define RNODE_INIT(name,c) struct rnode name =__RNODE_INIT(C_BEGIN ,NULL)
#define RNODE_BEGIN_DEF(name) RNODE_INIT(name,C_BEGIN)
#define RNODE_END_DEF(name) RNODE_INIT(name,C_END)


int main()
{
  RNODE_BEGIN_DEF(rbegin);
  RNODE_END_DEF(rend);

}
