SELECT nickname
FROM Trainer AS T, CatchedPokemon AS C,Pokemon AS P
WHERE T.id=C.owner_id AND P.id=C.pid
AND level>=50
ORDER BY nickname