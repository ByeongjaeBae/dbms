SELECT hometown AS City,AVG(level)
FROM Trainer AS T, CatchedPokemon AS C, Pokemon AS P
WHERE T.id=owner_id AND P.id=pid
GROUP BY hometown
ORDER BY AVG(level)