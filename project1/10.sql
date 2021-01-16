SELECT nickname
FROM Trainer AS T, CatchedPokemon AS C, Pokemon AS P
WHERE T.id=owner_id AND P.id=pid AND level>=50 
AND owner_id>=6
ORDER BY nickname