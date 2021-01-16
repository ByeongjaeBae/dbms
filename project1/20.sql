SELECT T.name,COUNT(*)
FROM Pokemon AS P, Trainer AS T, CatchedPokemon AS C
WHERE T.id=owner_id AND P.id=pid
AND hometown='Sangnok City'
GROUP BY T.name
ORDER BY COUNT(*)
