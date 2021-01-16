SELECT AVG(level)
FROM Trainer AS T, CatchedPokemon AS C,Pokemon AS P
WHERE T.id=C.owner_id AND P.id=C.pid
AND T.hometown='Sangnok City' AND P.type='Electric'