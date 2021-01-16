SELECT DISTINCT P.name
FROM Pokemon P,Trainer T,CatchedPokemon C
WHERE P.id=pid AND T.id=owner_id AND hometown='Sangnok City' AND P.name=ANY(
  SELECT P.name
  FROM Pokemon P,Trainer T,CatchedPokemon C
  WHERE P.id=pid AND T.id=owner_id AND hometown='Brown City'
  )
ORDER BY P.name