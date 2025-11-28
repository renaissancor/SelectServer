# 링 버퍼 로직 

Ring Buffer Info 

데이터 _head ~ _tail 

used size 는 
if(head < tail) return tail - head; 
else return head + capacity - tail; 

free size 는 
capacity - used size 


char *src 
int size 

를 받았을 경우 

size 를 firstSize, secondSize 로 나눈다 

firstDest = buffer + tail 부터 
buffer + tail + firstSize 까지 채운다. 

firstSize = min(firstSize, Capacity - Tail)

if(firstSize == size) 끝 

secondSize = size - firstSize; 

