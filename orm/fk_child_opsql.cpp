// foreign-one-many opsql.cpp — Automatically generated, do not manually modify
#include "orm/include/fk_child_opsql.h"
#include "orm/include/fk_child_base.h"
#include "models/include/FkChild.h"
#include "models/include/FkParent.h"

namespace orm {

template<> FkParent fk_child_opsql<FkChild, fk_child_base>::oneFkParent()
{
    return std::move(orm::FkParent()
        .where(
            orm::fk_parent_info::cols::id,
            orm::wq::eq,
            data.parent_id
        ));
}

template<> FkParent fk_child_opsql<FkChild, fk_child_base>::oneFkParent(int id)
{
    return std::move(orm::FkParent()
        .where(
            orm::fk_parent_info::cols::id,
            orm::wq::eq,
            id
        ));
}

template<> void fk_child_opsql<FkChild, fk_child_base>::oneFkParent(FkParent& m_obj)
{
        m_obj.where(
            orm::fk_parent_info::cols::id,
            orm::wq::eq,
            data.parent_id
        );
}

template<> FkParent fk_child_opsql<FkChild, fk_child_base>::manyFkParent()
{
    return std::move(orm::FkParent()
        .whereIn(
            orm::fk_parent_info::cols::id,
            get_cols_vec<fk_child_info::cols::parent_id>()
        ));
}

template<> void fk_child_opsql<FkChild, fk_child_base>::manyFkParent(FkParent& m_obj)
{
        m_obj.whereIn(
            orm::fk_parent_info::cols::id,
            get_cols_vec<fk_child_info::cols::parent_id>()
        );
}

} // namespace orm
