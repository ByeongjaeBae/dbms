SELECT T.name,AVG(level)
FROM Trainer AS T, CatchedPokemon AS C, Pokemon AS P,Gym AS G
WHERE T.id=owner_id AND P.id=pid AND T.id=leader_id
GROUP BY T.name
