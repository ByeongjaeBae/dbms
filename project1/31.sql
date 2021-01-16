SELECT type
FROM Pokemon P
WHERE P.id<>ALL(SELECT P.id
FROM Pokemon P
WHERE P.id<>ALL(
  SELECT P.id
  FROM Pokemon P,Evolution E
  WHERE P.id=before_id OR P.id=after_id)
 )
 GROUP BY type
 HAVING COUNT(*)>=3
 ORDER BY type DESC