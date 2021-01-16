SELECT T.name,MAX(level)
FROM Pokemon AS P,CatchedPokemon AS C,Trainer AS T
WHERE P.id=pid AND T.id=owner_id 
GROUP BY T.name
HAVING COUNT(*)>=4
ORDER BY T.name