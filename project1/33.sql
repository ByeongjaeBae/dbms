SELECT SUM(level)
FROM Pokemon P,Trainer T, CatchedPokemon C
WHERE P.id=pid AND T.name='Matis' AND T.id=owner_id 