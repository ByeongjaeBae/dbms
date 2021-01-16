SELECT T.name
FROM Trainer T
WHERE id<>ALL(
  SELECT leader_id
  FROM Trainer, Gym
  WHERE id=leader_id)
 ORDER BY T.name