tq

Timer queue implementation

tq is the prototype implementation of using one thread to control multiple timers (objects) the same time. When the solution per timer per thread costs too much (either because of too many timers or CPU power limitation), this 'not so accurate but works' solution is the best choice. K.R.K.C.

BUILD: gcc -o timer *.c -lpthread

Sep 26, 2013

root@davejingtian.org

http://davejingtian.org
