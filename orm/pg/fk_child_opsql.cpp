// foreign-one-many opsql.cpp — Automatically generated, do not manually modify
#include "orm/pg/include/fk_child_opsql.h"
#include "orm/pg/include/fk_child_base.h"
#include "models/pg/include/FkChild.h"
#include "models/pg/include/FkParent.h"

namespace orm { namespace pg {

template<> FkParent fk_child_opsql<FkChild, fk_child_base>::oneFkParent()
{
    return std::move(orm::pg::FkParent()
        .where(
            orm::pg::fk_parent_info::cols::id,
            orm::wq::eq,
            data.parent_id
        ));
}

template<> FkParent fk_child_opsql<FkChild, fk_child_base>::oneFkParent(int id)
{
    return std::move(orm::pg::FkParent()
        .where(
            orm::pg::fk_parent_info::cols::id,
            orm::wq::eq,
            id
        ));
}

template<> void fk_child_opsql<FkChild, fk_child_base>::oneFkParent(FkParent& m_obj)
{
        m_obj.where(
            orm::pg::fk_parent_info::cols::id,
            orm::wq::eq,
            data.parent_id
        );
}

template<> FkParent fk_child_opsql<FkChild, fk_child_base>::manyFkParent()
{
    return std::move(orm::pg::FkParent()
        .whereIn(
            orm::pg::fk_parent_info::cols::id,
            get_cols_vec<fk_child_info::cols::parent_id>()
        ));
}

template<> void fk_child_opsql<FkChild, fk_child_base>::manyFkParent(FkParent& m_obj)
{
        m_obj.whereIn(
            orm::pg::fk_parent_info::cols::id,
            get_cols_vec<fk_child_info::cols::parent_id>()
        );
}

}} // namespace orm::pg
