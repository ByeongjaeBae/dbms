SELECT P.name,level,nickname
FROM Pokemon P,CatchedPokemon C,Trainer T, Gym G
WHERE P.id=pid AND T.id=owner_id AND T.id =leader_id
AND nickname LIKE 'A%'
ORDER BY P.name DESC