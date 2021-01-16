SELECT P.name
FROM Pokemon P,(
  SELECT P.type,COUNT(*)CNT
  FROM Pokemon P
  GROUP BY P.type)A
 WHERE P.type=A.type AND
 (A.CNT=(
  SELECT MAX(A.CNT)
  FROM (
  SELECT COUNT(*)CNT
  FROM Pokemon P
  GROUP BY P.type)A
   )
   OR A.CNT=
  (SELECT CNT
  FROM (
  SELECT COUNT(*)CNT,P.type
  FROM Pokemon P
  GROUP BY P.type)A,
  (SELECT MAX(A.CNT)M
  FROM (
  SELECT COUNT(*)CNT
  FROM Pokemon P
  GROUP BY P.type)A
   )B
  WHERE A.CNT<>B.M
  ORDER BY CNT DESC
  LIMIT 1
  )
  )
 ORDER BY P.name