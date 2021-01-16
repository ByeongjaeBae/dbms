SELECT AVG(level)
FROM Trainer AS T, CatchedPokemon AS C, Pokemon AS P
WHERE T.name='Red' AND T.id=owner_id AND P.id=pid