SELECT COUNT(DISTINCT P.name)
FROM Pokemon AS P, Trainer AS T, CatchedPokemon AS C
WHERE T.id=owner_id AND P.id=pid
AND T.hometown='Sangnok City'
