SELECT T.name
FROM Pokemon AS P,CatchedPokemon AS C, Trainer AS T
WHERE T.id=owner_id AND P.id=pid 
GROUP BY T.name,P.name
HAVING COUNT(*)>=2
ORDER BY T.name