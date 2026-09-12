DROP TABLE IF EXISTS "fk_child";

CREATE TABLE "fk_child" (
  "id" SERIAL PRIMARY KEY,
  "parent_id" integer,
  "extra" text,
  CONSTRAINT "fk_child_parent_id_fkey" FOREIGN KEY ("parent_id") REFERENCES "fk_parent" ("id")
);
-- Note: sequence fk_child_id_seq created by SERIAL/BIGSERIAL automatically
