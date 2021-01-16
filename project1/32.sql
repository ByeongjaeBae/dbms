SELECT P.name
FROM Pokemon P
WHERE id<>ALL(
  SELECT pid
  FROM Pokemon P,CatchedPokemon C
  WHERE pid=P.id)
ORDER BY P.name