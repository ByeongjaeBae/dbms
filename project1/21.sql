SELECT T.name,COUNT(*)
FROM Pokemon AS P, Trainer AS T, CatchedPokemon AS C, Gym AS G
WHERE T.id=owner_id AND P.id=pid
AND T.id=leader_id
GROUP BY T.name
ORDER BY T.name