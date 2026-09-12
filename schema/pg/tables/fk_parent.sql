DROP TABLE IF EXISTS "fk_parent";

CREATE TABLE "fk_parent" (
  "id" SERIAL PRIMARY KEY,
  "name" text
);
-- Note: sequence fk_parent_id_seq created by SERIAL/BIGSERIAL automatically
