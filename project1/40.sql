SELECT T.hometown,nickname
FROM Trainer T,Pokemon P,CatchedPokemon C,(
  SELECT T.hometown,MAX(level) AS M
  FROM Trainer T,Pokemon P,CatchedPokemon C
  WHERE T.id=owner_id AND P.id=pid
  GROUP BY hometown
  )A
WHERE T.id=owner_id AND P.id=pid AND A.hometown=T.hometown AND M=level
ORDER BY T.hometown