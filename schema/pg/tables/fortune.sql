DROP TABLE IF EXISTS "fortune";

CREATE TABLE "fortune" (
  "id" SERIAL PRIMARY KEY,
  "message" varchar(2048)
);
-- Note: sequence fortune_id_seq created by SERIAL/BIGSERIAL automatically
