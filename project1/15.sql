SELECT T.id,cnt AS MAX
FROM Pokemon AS P,CatchedPokemon AS C, Trainer AS T,
(
  SELECT T.name,COUNT(*) AS cnt
  FROM Pokemon AS P,CatchedPokemon AS C, Trainer AS T
  WHERE T.id=owner_id AND P.id=pid
  GROUP BY T.name
  )A
WHERE T.id=owner_id AND P.id=pid AND A.name=T.name
AND cnt=(
  SELECT MAX(cnt)
  FROM (
      SELECT COUNT(*) AS cnt
  FROM Pokemon AS P,CatchedPokemon AS C, Trainer AS T
  WHERE T.id=owner_id AND P.id=pid
  GROUP BY T.name)A
  )
 GROUP BY T.id
ORDER BY T.id