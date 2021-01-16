# Disk-based b+tree
## Milestion01
### 1. Possible call path of the insert/delete operation
    * 도움말
    {} : 함수를 의미한다.
    
> # insert operation
~~~ 
     1. {insert}
        1. 집어넣을 노드를 찾아야 한다.
        2. {find}
            - 키값의 레코드를 찾는다. 그리고 duplicate의 여부도 찾는다.
            2-1 {find_leaf} 호출 - 말단 노드를 찾는다.
            2-2 리턴된 말단노드에서 키값을 탐색한다
            2-3 키값이 없으면 null을 리턴, 있을 경우 key값의 포인터를 **리턴한다.**
        3. {make_record}호출 - 레코드 생성
            3-1.value값을 넣어줄 레코드를 생성한다.
        4. root가 존재하지 않을 경우, 
          {start_new_tree}호출 -새로운 트리를 만든다
            4-1. 먼저 {make_leaf}를 호출한다.
                4-1-1.{make_node}호출
                    - node를 새로만든다.
                4- 만들어진 노드를 is_leaf값만 true로 설정해준다.
        5.  이미 트리가 있는 경우, 트리의 leaf를 {find_leaf}로 찾는다. (1-2-1-1)
        6. leaf의 키의 개수가 order-1보다 작을 경우,
           {insert_into_leaf}
            - leaf에 key값을 삽입
        7. leaf키의 개수가 order-1과 이미 같은 경우로 분할해줘야 한다.
        {insert_into_leaf_after_splitting} 그리고 리턴
            -leaf노드를 2개로 나눈다.
            7-1. {make_leaf} - new_leaf노드 생성
            7-2. 삽입할 key값과 node의 값들을 포함할 임시 배열 생성
            7-3. {cut} - 2개의 노드로 나눌 기준 인덱스이다.
                7-2-1. order가 짝수일 경우 /2 리턴
                7-2-2. 홀수일 경우 /2+1리턴
            7-4. new_leaf와 기존의 leaf에 key와 포인터를 나눈다.
            7-5. {insert_into_parent}
                - 자식노드를 분할 했으므로 parent의 값 삽입, leaf와 new_leaf를 자식으로 가지는 parent
                7-4-1. leaf의 parent가 없을 경우 ,
                       {insert_into_new_root} 그리고 리턴
                       - 인자로 받은 두 노드를 자식으로 하는 새로운 root 생성
                7-4-3. {get_left_index}
                        - left 노드가 부모노드의 포인터 배열에 index탐색
                7-4-4. parent의 키 개수가 order-1보다 작으면,
                       {insert_into_node} **리턴** 
                       - leaf노드가 아닌 노드에 삽입하는 것이다. **(이때 left는 leaf right는 new_leaf)**
                7-4-5. parent의 키 개수가 order-1보다 크거나 같으면,
                       {insert_into_node_after_splitting} **리턴** 
                       - parent노드를 분할해주고 새로운 상위 노드 만든다.
                       7-4-5-1. 삽입할 키값을 포함한 노드의 배열들을 만들기 
                       7-4-5-2. {cut}을 호출해 분할 할 인덱스 설정 split
                       7-4-5-3. {make_node}로 new_node 생성
                       7-4-5-4. old_node와 new_node에 위에서 만든 배열을 나누고 이때, split값을 통해 새로운 parent값인 k_prime 도출  
                       7-4-5-5. {insert_into_parent} **리턴** 
                        - 이 경우 k_prime에 값을 old_node와 new_node의 parent가 되는 node를만들고 그곳에 삽입
 ~~~
 > # delete operation
 ~~~
     2. {delete}
        1. 삭제할 레코드를 {find}로 찾는다 (
        2. 삭제할 키가 있는 leaf를 {find_leaf}로 찾는다.
        3. 제거할 key에 해당하는 레코드와 leaf노드가 전부 존재한다면,
           {delete_entry}
            3-1. {remove_entry_from_node}
                - node에서 key값을 제거하는 함수다.
                3-1-1. 포인터 배열과 key배열의 내용을 삭제하고 재조정해준다.
            3-2. leaf_node가 루트이면,
               {adjust_root}
                3-2-1. root에 key가 존재할경우 **root를 리턴**
                3-2-2. root에 키가 존재하지 않고, root가 leaf노드가 아니면 새로운 루트를 현재 root의 왼쪽 자식노드로 한다.
                3-2-3. leaf노드일경우 새로운 루트를 null로설정
            3-3. node의 최소사이즈를 설정해준다.
            3-4. 이때, 노드의 키의 개수가 최소사이즈보다 크거나 같으면 그대로 **root를 리턴**
            3-5. 아닐 경우, neighbor_index를 구해준다.
              {get_neigbor_index}호출
              - 자신의 왼쪽 이웃노드의 pointer에 해당하는 부모 포인터배열에 index 리턴
            3-6. neigbor_index에 따라 split하거나 merge할 key를 구한다.
            3-7. 최대용량을 설정해준다.
            3-8. 이때 이웃과 자신의 키의 수의 합이 용량보다 작으면
                {coalesce_nodes}를 통해 merge해준다. **리턴해준다. **- 2번 문항에서 기술
                  {delete_entry}
            3-9. 위에 합이 용량보다 크거나 같으면 재분배해준다.
                {redistributed_nodes} split해준다. **리턴해준다. ** -2번 문항에서 기술            
~~~