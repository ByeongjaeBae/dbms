SELECT P.name
FROM Pokemon P ,Evolution E
WHERE id=after_id AND id<>ALL(
  SELECT before_id
  FROM Evolution E
)
ORDER BY P.name