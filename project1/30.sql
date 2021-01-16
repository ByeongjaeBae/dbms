SELECT P1.id,P1.name,P2.name,P3.name
FROM Pokemon P1,Evolution E2,Evolution E,Pokemon P3,Pokemon P2
WHERE  P3.id=E.after_id AND E.before_id=ANY(
   SELECT E.after_id
    FROM Pokemon P ,Evolution E
   WHERE P.id=before_id)AND P3.id=E.after_id AND P3.id<>ALL(
  SELECT before_id
  FROM Evolution E)
  AND P2.id=E.before_id AND  P1.id<>ALL(
    SELECT P.id
    FROM Pokemon P ,Evolution E
    WHERE P.id=E.after_id 
  )
  AND P1.id=E2.before_id AND E2.after_id=ANY(
  SELECT before_id
  FROM Pokemon P ,Evolution E
  WHERE P.id=after_id) AND E2.after_id=E.before_id
ORDER BY P1.id