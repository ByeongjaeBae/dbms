SELECT COUNT(*)
FROM Pokemon P, CatchedPokemon C, Trainer T
WHERE P.id=pid AND T.id=owner_id
GROUP BY type
ORDER BY type 