SELECT COUNT(DISTINCT type)
FROM Pokemon AS P, Trainer AS T, CatchedPokemon AS C, Gym AS G
WHERE T.id=owner_id AND P.id=pid
AND T.id=leader_id AND city='Sangnok City'
