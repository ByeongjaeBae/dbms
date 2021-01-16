SELECT P.name
FROM Pokemon P,Evolution E
WHERE id=before_id AND id>after_id
ORDER BY P.name