SELECT T.name
FROM Pokemon P ,Evolution E,Trainer T, CatchedPokemon C
WHERE T.id=owner_id AND P.id=pid AND P.id=after_id AND P.id<>ALL(
  SELECT before_id
  FROM Evolution E
)
GROUP BY P.name,T.name
ORDER BY T.name
