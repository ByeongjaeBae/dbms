SELECT T.name,AVG(level)
FROM Trainer T,CatchedPokemon C, Pokemon P
WHERE T.id=owner_id AND P.id=pid AND (type='Normal' OR type='Electric')
GROUP BY T.name
ORDER BY AVG(level)