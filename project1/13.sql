SELECT P.name,P.id
FROM Trainer AS T, CatchedPokemon AS C, Pokemon AS P
WHERE T.id=owner_id AND P.id=pid
AND T.hometown='Sangnok City'
ORDER BY P.id