// foreign-one-many opsql.cpp — Automatically generated, do not manually modify
#include "orm/include/fk_parent_opsql.h"
#include "orm/include/fk_parent_base.h"
#include "models/include/FkParent.h"
#include "models/include/FkChild.h"

namespace orm {

template<> FkChild fk_parent_opsql<FkParent, fk_parent_base>::oneFkChild()
{
    return std::move(orm::FkChild()
        .where(
            orm::fk_child_info::cols::parent_id,
            orm::wq::eq,
            data.id
        ));
}

template<> FkChild fk_parent_opsql<FkParent, fk_parent_base>::oneFkChild(int id)
{
    return std::move(orm::FkChild()
        .where(
            orm::fk_child_info::cols::parent_id,
            orm::wq::eq,
            id
        ));
}

template<> void fk_parent_opsql<FkParent, fk_parent_base>::oneFkChild(FkChild& m_obj)
{
        m_obj.where(
            orm::fk_child_info::cols::parent_id,
            orm::wq::eq,
            data.id
        );
}

template<> FkChild fk_parent_opsql<FkParent, fk_parent_base>::manyFkChild()
{
    return std::move(orm::FkChild()
        .whereIn(
            orm::fk_child_info::cols::parent_id,
            get_cols_vec<fk_parent_info::cols::id>()
        ));
}

template<> void fk_parent_opsql<FkParent, fk_parent_base>::manyFkChild(FkChild& m_obj)
{
        m_obj.whereIn(
            orm::fk_child_info::cols::parent_id,
            get_cols_vec<fk_parent_info::cols::id>()
        );
}

} // namespace orm
