# Disk-based b+tree
## Milestion01
### 2. Detail flow of the structure modification (split, merge)
    * 도움말
    {} : 함수를 의미한다.
    
> # Merge
  ##  {coalesce_nodes}
~~~
    
    Merge 조건 : 1. 레코드를 제거하려는 위치에 node와 그의 sibling_node의 
    num_key의 합이 capacity보다 작을때이다.
    
        1. leaf node가 아닌 internal node에서 merge되는 경우
            1. 부모노드에서 삭제할 레코드에 해당하는 index에 값을 left노드로 가져온다.
            2. 삭제할 레코드를 삭제하고, sibling노드(right)에 있는 레코드를 가져온다.
            3. right의 마지막 포인터를 left로 가져온다. /항상 key의 수보다 pointer의 수가 더 많다.
            4. right와 left의 자식노드들을 부모로 left를 가리키게 만든다.
            5. sibling노드를 메모리 해제한다.
        2. leaf노드에서 merge되는 경우
            1. 삭제할 레코드를 삭제하고, sibling노드(right)에 있는 레코드를 모두 가져온다.
            2. pointer로 다음 sibling을 가리키게 한다.
            3. sibling노드를 메모리 해제한다.
~~~
  ## {redistributed_nodes}
~~~
    Merge 조건 : 2. 레코드를 제거하려는 위치에 node와 그의 sibling_node의 
    num_key의 합이 capacity보다 크거나 같을때 이다.
        1. node가 leftmost node가 아닐때
            1. n이 leaf node가 아닐때
                1.  마지막 포인터를 1칸 오른쪽으로 옮긴다.
                2. node의 값을 전부 오른쪽으로 1칸씩 옮기고, node에 해당하는 parent의 key값을 받는다.(k_prime)
                3. node에 해당하는 parent의 index에(k_prime_index) sibling(left)에 마지막 값을 넣어준다.
            2. n이 leaf node일때
                1. node의 값을 전부 오른쪽으로 1칸씩 옮기고, sibling(left)의 마지막 값을 받는다.
                2. node에 해당하는 parent의 index에(k_prime_index) node의 첫 값을 넣는다.
        2. node가 leftmost node일 경우
            1. n이 leaf node일 경우
                1. node의 마지막 값을 sibling(right)의 첫 값을 받는다.
                2. node에 해당하는 parent의 index에(k_prime_index)에 sibling의 남은 2번째 값을 넣는다.
                3. sibling의 값을 1칸씩 앞으로 조정한다.
            2. n이 leaf node가 아닐 경우 
                1. node의 마지막 값에 node에 해당하는 parent의 key값을 받는다.(k_prime)
                2. sibling의 첫 값에 해당하는 포인터를 받고, 그 자식의 부모를 node로 설정한다.
                3. node에 해당하는 parent의 index에(k_prime_index)에 sibling의 첫 값을 받는다.
                4. sibling의 값을 1칸씩 앞으로 조정한다.
                5. sibling의 포인터를 1칸 더 조정한다.
~~~
 > # Split
 ~~~
    Split 조건 : 레코드를 추가하는 위치의 leaf노드가 full일때 한다. 
        1. leaf노드만 split할 경우
            1. 임시배열을 선언하고 삽입지점을 찾고 삽입지점에 레코드를 넣는다.
            2. leaf노드를 절반으로 나눈다.
            3. 절반을 기존 leaf노드로 나머지를 new_leaf노드로 옮겨준다.
            4. new_leaf의 가장왼쪽 값을 split시킨 부모노드의 넣는다.
        2. internal node일 경우 - leaf노드에서 split되서 올라가면 거기서 검사 즉 재귀적으로 반복
             1. 임시배열을 선언하고 삽입지점을 찾고 삽입지점에 레코드를 넣는다.
             2. internal 노드를 절반으로 나눈다.
             3. 절반은 기존 internal노드로 나머지는 new_internal 노드에 넣어준다.
             4. 이때, 절반이 되는 지점은 internal노드에게 넣지않는다.
             5. 넣지 않은값을 split 시킨 부모노드에 넣는다.
        3. root node일 경우 - internal노드가 split되서 올라가면 거기서 검사를 한다. 재귀적이다.
            1. 임시배열을 선언하고 삽입지점을 찾고 삽입지점에 레코드를 넣는다.
            2. root 노드를 절반으로 나눈다.
            3. 절반은 기존 root노드로 나미저를 new_node로 넣는다.
            4. 이때, 절반이 되는 지점은 internal노드에게 넣지않는다.
            5. 이때, parent 노드가 없으므로 새로운 root노드를 만든다.
            6. 새로운 root노드에  넣지않은 값을 넣는다.
~~~
