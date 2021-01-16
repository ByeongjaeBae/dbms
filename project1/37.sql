SELECT T.name,S
FROM Pokemon P,Trainer T,CatchedPokemon C,(
  SELECT T.name,SUM(level) AS S
  FROM Pokemon P,Trainer T,CatchedPokemon C
  WHERE P.id=pid AND T.id=owner_id
  GROUP BY T.name
 )A
WHERE P.id=pid AND T.id=owner_id AND T.name=A.name
AND S=(
  SELECT MAX(S)
  FROM (
  SELECT T.name,SUM(level) AS S
  FROM Pokemon P,Trainer T,CatchedPokemon C
  WHERE P.id=pid AND T.id=owner_id
  GROUP BY T.name
 )A
  )
  GROUP BY T.name
  ORDER BY T.name