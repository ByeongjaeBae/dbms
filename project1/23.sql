SELECT DISTINCT T.name
FROM Pokemon AS P, Trainer AS T, CatchedPokemon AS C
WHERE T.id=owner_id AND P.id=pid
 AND level<=10
ORDER BY T.name
