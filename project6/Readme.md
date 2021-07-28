# Crash Recovery

이번 과제는 Recovery 구현을 위한 3 pass recovery의 구현이 목표이다.

## 1, Definition

### 1.1. Transaction

Transaction (이하 TRX) 은 User Defined Instruction Sequence로, ACID의 속성을 가진다.

- Atomicity: 전체 TRX이 실행에 성공하거나, TRX 자체가 실행되지 않는다.
- Consistency: 읽고 쓰는 값의 일관성이 보장된다.
- Isolation: 사용자가 다른 사용자로부터 시스템이 독립되었다 느낀다.
- Durability: 실행에 성공한 Transaction에 대한 결과가 영속적으로 보존된다.

#### 1.1.1. Atomicity

커밋 또는 abort로 정상적으로 끝나지 않은 것들은 전부 undo처리해줘야 한다.

#### 1.1.2. Durability

커밋이나 abort가 된 것들은 무조건 redo처리를 통해 보존해줘야 한다.


#### 2.1. Analyze

트랜잭션을 winner와 loser 트랜잭션으로 나누는 과정이다.
트랜잭션이 commit되거나 abort 되었을 경우가 winner 트랜잭션이고
중간에 system crash로 인해 멈췄을 경우 loser 트랜잭션이다.

#### 2.2. Redo

redo는 기본적으로 모든 트랜잭션들은 redo해준다.
간단하게 commit 된거는 commit 로그 abort된거는 rollback 로그로써 redo를 진행해준다.
모든 트랜잭션에 대해서 로그파일을 처음부터 순회하면서 redo를 해준다.
redo 작업이 끝나게 되면, 데이터베이스 상태는 장애 발생 시점의 상태와 같게 된다.
중간에 redo_crash가 일어날 경우 이전에 한번 redo가 진행되었던것은 가장 최근 page lsn을 기준으로
해당 로그의 lsn과 page lsn을 비교함으로써 판단할 수 있다.
결과적으로 page lsn이 해당 로그의 lsn보다 같거나 크면 복구가 필요치 않다.
이러한 consider_redo를 진행하게 해야한다. 
consider_redo를 통해 fast recovery가 가능하게 된다.

#### 2.3. Undo

undo는 loser 트랜잭션들을 마치 일어나지 않은 것처럼 원래 상태로 돌려주는 것이다.
기본적으로 undo는 역방향으로 탐색하면서 undo 복구가 필요한 로그들에 대해서 undo 복구를 수행한다.
undo pass는 redo pass 이후에 진행이 된다. 최초의 undo pass는 순서대로 undo를 진행하게 된다.
undo를 수행하고 나면 해당 undo작업에 대한 clr을 발급한다. redo전용 로그로써, 
undo를 하고 난 이후에 다시 undo를 하는 행위가 일어나지 않기 위해 필요하다.
clr은 이전 로그 레코드 위치를 undo로그의 이전 로그를 가리키도록 한다. 이런식으로 이전 로그를 계속 탐색한다.
근데 이때 undo_crash로 인해 undo중 fail하게 될 경우, clr을 통해 이미 undo 된 것들은 스킵하고,
새로 undo해 줄 것들을 복구한다.


