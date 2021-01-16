SELECT P.name
FROM Pokemon AS P,CatchedPokemon AS C
WHERE P.id=pid AND nickname Like'% %'
ORDER BY P.name DESC