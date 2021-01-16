SELECT DISTINCT P.name,P.type
FROM Trainer AS T, CatchedPokemon AS C, Pokemon AS P
WHERE T.id=owner_id AND P.id=pid AND level>=30
ORDER BY P.name