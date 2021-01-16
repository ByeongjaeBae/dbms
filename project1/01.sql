SELECT T.name
FROM Trainer AS T,CatchedPokemon AS C,Pokemon AS P
WHERE T.id=owner_id AND p.id=pid
GROUP BY T.name
HAVING COUNT(*)>=3
ORDER BY COUNT(*) DESC;